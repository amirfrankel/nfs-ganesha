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
 * \file    mnt_Null.c
 * \author  $Author: deniel $
 * \date    $Date: 2005/12/20 10:52:14 $
 * \version $Revision: 1.8 $
 * \brief   MOUNTPROC_NULL for Mount protocol v1 and v3.
 *
 * mnt_Null.c : MOUNTPROC_NULL in V1, V3.
 *
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>           /* for having FNDELAY */
#include "HashTable.h"
#include "log.h"
#include "nfs23.h"
#include "nfs4.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "nfs_exports.h"
#include "nfs_creds.h"
#include "nfs_tools.h"
#include "mount.h"
#include "nfs_proto_functions.h"

/**
 * @brief The Mount proc null function, for all versions.
 *
 * The MOUNT proc null function, for all versions.
 *
 * @param[in]  parg     ignored
 * @param[in]  pexport  ignored
 * @param[in]  pcontext ignored
 * @param[in]  pclient  ignored
 * @param[in]  preq     ignored
 * @param[out] pres     ignored
 *
 */

int mnt_Null(nfs_arg_t *parg,
             exportlist_t *pexport,
	     struct req_op_context *req_ctx,
             nfs_worker_data_t *pworker,
             struct svc_req *preq,
             nfs_res_t *pres)
{
  LogDebug(COMPONENT_NFSPROTO, "REQUEST PROCESSING: Calling mnt_Null");
  return MNT3_OK;
}                               /* mnt_Null */

/**
 * mnt_Null_Free: Frees the result structure allocated for mnt_Null
 * 
 * Frees the result structure allocated for mnt_Null. Does Nothing in fact.
 * 
 * @param pres        [INOUT]   Pointer to the result structure.
 *
 */
void mnt_Null_Free(nfs_res_t * pres)
{
  return;
}                               /* mnt_Export_Free */
