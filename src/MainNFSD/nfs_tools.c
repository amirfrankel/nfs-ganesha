/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ---------------------------------------
 */

/**
 * \file    nfs_tools.c
 * \author  $Author: deniel $
 * \date    $Date: 2006/01/20 07:39:22 $
 * \version $Revision: 1.43 $
 * \brief   Some tools very usefull in the nfs protocol implementation.
 *
 * nfs_tools.c : Some tools very usefull in the nfs protocol implementation
 *
 *
 */
#include "config.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef FREEBSD
#include <netinet/tcp.h>
#endif                          /* _FREEBSD */
#include <sys/types.h>
#include <ctype.h>              /* for having isalnum */
#include <stdlib.h>             /* for having atoi */
#include <dirent.h>             /* for having MAXNAMLEN */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>           /* for having FNDELAY */
#include <pwd.h>

#include <grp.h>
#include "log.h"
#include "ganesha_rpc.h"
#include "nfs_core.h"
#include "nfs23.h"
#include "nfs4.h"
#include "fsal.h"
#include "nfs_tools.h"
#include "nfs_exports.h"
#include "nfs_file_handle.h"

struct tcp_conn
{                               /* kept in xprt->xp_p1 */
  enum xprt_stat strm_stat;
  u_long x_id;
  XDR xdrs;
  char verf_body[MAX_AUTH_BYTES];
};

unsigned long decimal_simple_hash_func(hash_parameter_t * p_hparam,
                                       struct gsh_buffdesc * buffclef)
{
  LogMajor(COMPONENT_MAIN, "ATTENTION: CALLING A DUMMY FUNCTION");
  return 0;
}

unsigned long decimal_rbt_hash_func(hash_parameter_t * p_hparam, struct gsh_buffdesc * buffclef)
{
  LogMajor(COMPONENT_MAIN, "ATTENTION: CALLING A DUMMY FUNCTION");
  return 0;
}

int display_cache(struct gsh_buffdesc * pbuff, char *str)
{
    *str = '\0';
  return 0;
}

int compare_cache(struct gsh_buffdesc * buff1, struct gsh_buffdesc * buff2)
{
  return 0;
}
