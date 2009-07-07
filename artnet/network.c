/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * network.c
 * Network code for libartnet
 * Copyright (C) 2004-2007 Simon Newton
 *
 */

#include <errno.h>
#include <sys/socket.h> // socket before net/if.h for mac
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "private.h"

enum { INITIAL_IFACE_COUNT = 10 };
enum { IFACE_COUNT_INC = 5 };
enum { IFNAME_SIZE = 32 }; // 32 sounds a reasonable size


typedef struct iface_s {
  struct sockaddr_in ip_addr;
  struct sockaddr_in bcast_addr;
  int8_t hw_addr[ARTNET_MAC_SIZE];
  char   if_name[IFNAME_SIZE];
  struct iface_s *next;
} iface_t;

unsigned long LOOPBACK_IP = 0x7F000001;


#ifdef HAVE_GETIFADDRS
 #ifdef HAVE_LINUX_IF_PACKET_H
   #define USE_GETIFADDRS
 #endif
#endif

#ifdef USE_GETIFADDRS
  #include <ifaddrs.h>
  #include <linux/types.h> // required by if_packet
  #include <linux/if_packet.h>
#endif


// free memory used by the iface's list
static void free_ifaces(iface_t *head) {
  iface_t *ift, *ift_next;

  for (ift = head; ift != NULL; ift = ift_next) {
    ift_next = ift->next;
    free(ift);
  }
}

#ifdef USE_GETIFADDRS

/*
 * Check if we are interested in this interface
 * @param ifa  pointer to a ifaddr struct
 */
static iface_t *check_iface(struct ifaddrs *ifa) {
  struct sockaddr_in *sin;
  iface_t *ret = 0;

  if (!ifa || !ifa->ifa_addr) return 0;

  // skip down, loopback and non inet interfaces
  if (!(ifa->ifa_flags & IFF_UP)) return 0;
  if (ifa->ifa_flags & IFF_LOOPBACK) return 0;
  if (ifa->ifa_addr->sa_family != AF_INET) return 0;

  ret = calloc(1, sizeof(iface_t));
  if (ret == NULL) {
    artnet_error("%s: calloc error %s" , __FUNCTION__, strerror(errno));
    return 0;
  }

  sin = (struct sockaddr_in*) ifa->ifa_addr;
  ret->ip_addr.sin_addr = sin->sin_addr;
  strncpy(ret->if_name, ifa->ifa_name, IFNAME_SIZE-1);

  if (ifa->ifa_flags & IFF_BROADCAST) {
    sin = (struct sockaddr_in *) ifa->ifa_broadaddr;
    ret->bcast_addr.sin_addr = sin->sin_addr;
  }
  return ret;
}


/*
 * Returns a linked list of interfaces on this machine using getifaddrs
 *  loopback interfaces are skipped as well as interfaces which are down
 *
 * @param ift_head_r address of the pointer to the head of the list
 */
static int get_ifaces(iface_t **ift_head_r) {
  struct ifaddrs *ifa, *ifa_iter;
  iface_t *if_head, *if_tmp, *if_iter;
  struct sockaddr_ll *sll;
  char *if_name, *cptr;

  if (getifaddrs(&ifa) != 0) {
    artnet_error("Error getting interfaces: %s", strerror(errno));
    return ARTNET_ENET;
  }

  if_head = 0;
  if_iter = 0;
  for (ifa_iter = ifa; ifa_iter != NULL; ifa_iter = ifa_iter->ifa_next) {
    if_tmp = check_iface(ifa_iter);
    if (if_tmp) {
      // We got new usable interface
      if (!if_iter) {
        if_head = if_iter = if_tmp;
      } else {
        if_iter->next = if_tmp;
        if_iter = if_tmp;
      }
    }
  }


  // Match up the interfaces with the corrosponding AF_PACKET interface
  // to fetch the hw addresses
  //
  // TODO: Will probably not work on OS X, it should
  //      return AF_LINK -type sockaddr
  for (if_iter = if_head; if_iter != NULL; if_iter = if_iter->next) {
    if_name = strdup(if_iter->if_name);

    // if this is an alias, get mac of real interface
    if ((cptr = strchr(if_name, ':')))
      *cptr = 0;

    // Find corresponding iface_t -structure
    for (ifa_iter = ifa; ifa_iter != NULL; ifa_iter = ifa_iter->ifa_next) {
      if ((! ifa_iter->ifa_addr) || ifa_iter->ifa_addr->sa_family  != AF_PACKET)
        continue;

      if (strncmp(if_name, ifa_iter->ifa_name, IFNAME_SIZE) == 0) {
        // Found matching hw-address
        sll = (struct sockaddr_ll*) ifa_iter->ifa_addr;
        memcpy(if_iter->hw_addr, sll->sll_addr, ARTNET_MAC_SIZE);
        break;
      }
    }
    free(if_name);
  }

  *ift_head_r = if_head;
  freeifaddrs(ifa);
  return 0;
}

#else

/*
 *
 * Returns a linked list of interfaces on this machine using ioctl's
 *  loopback interfaces are skipped as well as interfaces which are down
 *
 * @param ift_head_r address of the pointer to the head of the list
 */
static int get_ifaces(iface_t **ift_head_r) {
  struct ifconf ifc;
  struct ifreq *ifr, ifrcopy;
  struct sockaddr_in *sin;
  int len, lastlen, flags;
  char *buf, *ptr;
  iface_t *ift_head, **ift_next, *ift;
  int ret = ARTNET_EOK;
  int sd;

  ift_head = NULL;
  ift_next = &ift_head;

  // create socket to get iface config
  sd = socket(PF_INET, SOCK_DGRAM, 0);

  if (sd < 0) {
    artnet_error("%s : Could not create socket %s", __FUNCTION__, strerror(errno));
    ret = ARTNET_ENET;
    goto e_return;
  }

  // first use ioctl to get a listing of interfaces
  lastlen = 0;
  len = INITIAL_IFACE_COUNT * sizeof(struct ifreq);

  for (;;) {
    buf = malloc(len);

    if (buf == NULL) {
      artnet_error_malloc();
      ret = ARTNET_EMEM;
      goto e_free;
    }

    ifc.ifc_len = len;
    ifc.ifc_buf = buf;
    if (ioctl(sd, SIOCGIFCONF, &ifc) < 0) {
      if (errno != EINVAL || lastlen != 0) {
        artnet_error("%s : ioctl error %s", __FUNCTION__, strerror(errno));
        ret = ARTNET_ENET;
        goto e_free;
      }
    } else {
      if (ifc.ifc_len == lastlen)
        break;
      lastlen = ifc.ifc_len;
    }
    len += IFACE_COUNT_INC * sizeof(struct ifreq);
    free(buf);
  }

  // loop through each iface
  for (ptr = buf; ptr < buf + ifc.ifc_len;) {

    ifr = (struct ifreq*) ptr;

    // work out length here
#ifdef HAVE_SOCKADDR_SA_LEN

    len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);

#else
    switch (ifr->ifr_addr.sa_family) {
#ifdef  IPV6
      case AF_INET6:
        len = sizeof(struct sockaddr_in6);
        break;
#endif
      case AF_INET:
      default:
        len = sizeof(SA);
        break;
    }
#endif

    ptr += sizeof(ifr->ifr_name) + len;

    // look for AF_INET interfaces
    if (ifr->ifr_addr.sa_family == AF_INET) {
      ifrcopy = *ifr;
      if (ioctl(sd, SIOCGIFFLAGS, &ifrcopy) < 0) {
        artnet_error("%s : ioctl error %s" , __FUNCTION__, strerror(errno));
        ret = ARTNET_ENET;
        goto e_free_list;
      }

      flags = ifrcopy.ifr_flags;
      if ((flags & IFF_UP) == 0)
        continue; //skip down interfaces

      if ((flags & IFF_LOOPBACK))
        continue; //skip lookback

      // interesting iface, better malloc for it ..
      ift = calloc(1, sizeof(iface_t));

      if (ift == NULL) {
        artnet_error("%s : calloc error %s" , __FUNCTION__, strerror(errno));
        ret = ARTNET_EMEM;
        goto e_free_list;
      }

      if (ift_head == NULL) {
        ift_head = ift;
      } else {
        *ift_next = ift;
      }

      ift_next = &ift->next;

      sin = (struct sockaddr_in *) &ifr->ifr_addr;
      ift->ip_addr.sin_addr = sin->sin_addr;

      // fetch bcast address
#ifdef SIOCGIFBRDADDR
      if (flags & IFF_BROADCAST) {
        if (ioctl(sd, SIOCGIFBRDADDR, &ifrcopy) < 0) {
          artnet_error("%s : ioctl error %s" , __FUNCTION__, strerror(errno));
          ret = ARTNET_ENET;
          goto e_free_list;
        }

        sin = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;
        ift->bcast_addr.sin_addr = sin->sin_addr;
      }
#endif
      // fetch hardware address
#ifdef SIOCGIFHWADDR
      if (flags & SIOCGIFHWADDR) {
        if (ioctl(sd, SIOCGIFHWADDR, &ifrcopy) < 0) {
          artnet_error("%s : ioctl error %s", __FUNCTION__, strerror(errno));
          ret = ARTNET_ENET;
          goto e_free_list;
        }
        memcpy(&ift->hw_addr, ifrcopy.ifr_hwaddr.sa_data, ARTNET_MAC_SIZE);
      }
#endif

    /* ok, if that all failed we should prob try and use sysctl to work out the bcast
     * and hware addresses
     * i'll leave that for another day
     */
    } else {
      //
    }
  }
  *ift_head_r = ift_head;
  free(buf);
  return ARTNET_EOK;

e_free_list:
  free_ifaces(ift_head);
e_free:
  free(buf);
  close(sd);
e_return:
  return ret;
}

#endif


/*
 * Scan for interfaces, and work out which one the user wanted to use.
 *
 */
int artnet_net_init(node n, const char *preferred_ip) {
  iface_t *ift, *ift_head = NULL;
  struct in_addr wanted_ip;

  int found = FALSE;
  int i;
  int ret = ARTNET_EOK;

  if ((ret = get_ifaces(&ift_head)))
    goto e_return;

  if (n->state.verbose) {
    printf("#### INTERFACES FOUND ####\n");
    for (ift = ift_head; ift != NULL; ift = ift->next) {
      printf("IP: %s\n", inet_ntoa(ift->ip_addr.sin_addr));
      printf("  bcast: %s\n" , inet_ntoa(ift->bcast_addr.sin_addr));
      printf("  hwaddr: ");
      for (i = 0; i < ARTNET_MAC_SIZE; i++) {
        printf("%hhx:", ift->hw_addr[i]);
      }
      printf("\n");
    }
    printf("#########################\n");
  }

  if (preferred_ip) {
    // search through list of interfaces for one with the correct address
    ret = artnet_net_inet_aton(preferred_ip, &wanted_ip);
    if (ret)
      goto e_cleanup;

    for (ift = ift_head; ift != NULL; ift = ift->next) {
      if (ift->ip_addr.sin_addr.s_addr == wanted_ip.s_addr) {
        found = TRUE;
        n->state.ip_addr = ift->ip_addr.sin_addr;
        n->state.bcast_addr = ift->bcast_addr.sin_addr;
        memcpy(&n->state.hw_addr, &ift->hw_addr, ARTNET_MAC_SIZE);
        break;
      }
    }
    if (!found) {
      artnet_error("Cannot find ip %s", preferred_ip);
      ret = ARTNET_ENET;
      goto e_cleanup;
    }
  } else {
    if (ift_head) {
      // pick first address
      // copy ip address, bcast address and hardware address
      n->state.ip_addr = ift_head->ip_addr.sin_addr;
      n->state.bcast_addr = ift_head->bcast_addr.sin_addr;
      memcpy(&n->state.hw_addr, &ift_head->hw_addr, ARTNET_MAC_SIZE);
    } else {
      artnet_error("No interfaces found!");
      ret = ARTNET_ENET;
    }
  }

e_cleanup:
  free_ifaces(ift_head);
e_return :
  return ret;
}


/*
 * Commence listening on the sockets
 *
 */
int artnet_net_start(node n) {
  struct sockaddr_in servAddr;
  int bcast_flag = TRUE;
  node tmp;
  int ret = ARTNET_EOK;

  // only attempt to bind to the broadcast if we are the group master
  if (n->peering.master == TRUE) {

    /* create socket */
    n->sd = socket(PF_INET, SOCK_DGRAM, 0);

    if (n->sd < 0) {
      artnet_error("Could not create socket %s", strerror(errno));
      ret = ARTNET_ENET;
      goto e_socket1;
    }

    memset(&servAddr, 0x00, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(ARTNET_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (n->state.verbose)
      printf("Binding to %s \n" , inet_ntoa(servAddr.sin_addr));

    /* bind sockets
     * we do one for the ip address and one for the broadcast address
     */
    if (bind(n->sd, (SA *) &servAddr, sizeof(servAddr)) == -1) {
      artnet_error("Failed to bind to socket %s", strerror(errno));
      ret = ARTNET_ENET;
      goto e_bind1;
    }

    // allow bcasting
    if (setsockopt(n->sd, SOL_SOCKET, SO_BROADCAST, &bcast_flag, sizeof(int)) == -1) {
      artnet_error("Failed to bind to socket %s", strerror(errno));
      ret =  ARTNET_ENET;
      goto e_setsockopt;
    }

    // now we need to propagate the sd to all our peers
    for(tmp = n->peering.peer; tmp != NULL && tmp != n; tmp = tmp->peering.peer)
      tmp->sd = n->sd;
  }

  return ARTNET_EOK;

e_setsockopt:
e_bind1:
  close(n->sd);

e_socket1:
  return ret;
}


/*
 * Recv a packet from the network
 *
 *
 */
int artnet_net_recv(node n, artnet_packet p, int delay) {
  ssize_t len;
  struct sockaddr_in cliAddr;
  socklen_t cliLen = sizeof(cliAddr);
  fd_set rset;
  struct timeval tv;
  int maxfdp1 = n->sd + 1;

  FD_ZERO(&rset);
  FD_SET((unsigned int) n->sd, &rset);

  tv.tv_usec = 0;
  tv.tv_sec = delay;
  p->length = 0;

  switch (select(maxfdp1, &rset, NULL, NULL, &tv)) {
    case 0:
      // timeout
      return RECV_NO_DATA;
      break;
    case -1:
      if ( errno != EINTR) {
        artnet_error("%s : select error", __FUNCTION__);
        return ARTNET_ENET;
      }
      return ARTNET_EOK;
      break;
    default:
      break;
  }

  // need a check here for the amount of data read
  // should prob allow an extra byte after data, and pass the size as sizeof(Data) +1
  // then check the size read and if equal to size(data)+1 we have an error
  len = recvfrom(n->sd, &(p->data), sizeof(p->data), 0, (SA*) &cliAddr, &cliLen);
  if (len < 0) {
    artnet_error("%s : recvfrom error %s", __FUNCTION__, strerror(errno));
    return ARTNET_ENET;
  }

  if (cliAddr.sin_addr.s_addr == n->state.ip_addr.s_addr ||
      ntohl(cliAddr.sin_addr.s_addr) == LOOPBACK_IP) {
    p->length = 0;
    return ARTNET_EOK;
  }

  p->length = len;
  memcpy(&(p->from), &cliAddr.sin_addr, sizeof(struct in_addr));
  // should set to in here if we need it
  return ARTNET_EOK;

}


/*
 * Send a packet to the network
 *
 *
 */
int artnet_net_send(node n, artnet_packet p) {
  struct sockaddr_in addr;
  int ret;

  if (n->state.mode != ARTNET_ON)
    return ARTNET_EACTION;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(ARTNET_PORT);
  addr.sin_addr = p->to;
  p->from = n->state.ip_addr;

  if (n->state.verbose)
    printf("sending to %s\n" , inet_ntoa(addr.sin_addr));

  ret = sendto(n->sd, &p->data, p->length, 0, (SA*) &addr, sizeof(addr));
  if (ret == -1) {
    artnet_error("Sendto failed: %s", strerror(errno));
    n->state.report_code = ARTNET_RCUDPFAIL;
    return ARTNET_ENET;

  } else if (p->length != ret) {
    artnet_error("failed to send full datagram");
    n->state.report_code = ARTNET_RCSOCKETWR1;
    return ARTNET_ENET;
  }

  if (n->callbacks.send.fh != NULL) {
    get_type(p);
    n->callbacks.send.fh(n, p, n->callbacks.send.data);
  }
  return ARTNET_EOK;
}


/*
int artnet_net_reprogram(node n) {
  iface_t *ift_head, *ift;
  int i;

  ift_head = get_ifaces(n->sd[0]);

  for (ift = ift_head;ift != NULL; ift = ift->next ) {
    printf("IP: %s\n", inet_ntoa(ift->ip_addr.sin_addr) );
    printf("  bcast: %s\n" , inet_ntoa(ift->bcast_addr.sin_addr) );
    printf("  hwaddr: ");
      for(i = 0; i < 6; i++ ) {
        printf("%hhx:", ift->hw_addr[i] );
      }
    printf("\n");
  }

  free_ifaces(ift_head);

}*/


int artnet_net_set_fdset (node n, fd_set *fdset) {
  FD_SET((unsigned int) n->sd, fdset);
  return ARTNET_EOK;
}


int artnet_net_close(node n) {
  if (close(n->sd)) {
    artnet_error(strerror(errno));
    return ARTNET_ENET;
  }
  return ARTNET_EOK;
}


/*
 * Convert a string to an in_addr
 */
int artnet_net_inet_aton(const char *ip_address, struct in_addr *address) {
#ifdef HAVE_INET_ATON
  if (!inet_aton(ip_address, address)) {
#else
  if ((*address = inet_addr(ip_address)) == INADDR_NONE) {
#endif
    artnet_error("IP conversion from %s failed", ip_address);
    return ARTNET_EARG;
  }
  return ARTNET_EOK;
}

