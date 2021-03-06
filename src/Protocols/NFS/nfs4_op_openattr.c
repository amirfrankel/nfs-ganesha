/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * ---------------------------------------
 */

/**
 * \file    nfs4_op_openattr.c
 * \brief   Routines used for managing the NFS4 COMPOUND functions.
 *
 * Routines used for managing the NFS4 COMPOUND functions.
 *
 * $Header: /cea/home/cvs/cvs/SHERPA/BaseCvs/GANESHA/src/NFS_Protocols/nfs4_op_openattr.c,v 1.8 2005/11/28 17:02:51 deniel Exp $
 *
 */
#include "config.h"
#include "HashTable.h"
#include "log.h"
#include "nfs4.h"
#include "nfs_core.h"
#include "cache_inode.h"
#include "nfs_proto_functions.h"
#include "nfs_tools.h"

/**
 *
 * @brief NFS4_OP_OPENATTR
 *
 * This function implements the NFS4_OP_OPENATTRR operation.
 *
 * @param[in]     op   Arguments for nfs4_op
 * @param[in,out] data Compound request's data
 * @param[out]    resp Results for nfs4_op
 *
 * @return per RFC5661, pp. 370-1
 *
 */
#define arg_OPENATTR4 op->nfs_argop4_u.opopenattr
#define res_OPENATTR4 resp->nfs_resop4_u.opopenattr

int nfs4_op_openattr(struct nfs_argop4 *op,
                     compound_data_t *data,
                     struct nfs_resop4 *resp)
{
  resp->resop = NFS4_OP_OPENATTR;
  res_OPENATTR4.status = NFS4_OK;

  res_OPENATTR4.status = nfs4_fh_to_xattrfh(&(data->currentFH),
                                            &(data->currentFH));

  return res_OPENATTR4.status;
}                               /* nfs4_op_openattr */

/**
 * @brief Free memory allocated for OPENATTR result
 *
 * This function frees any memory allocated for the result of the
 * NFS4_OP_OPENATTR operation.
 *
 * @param[in,out] resp nfs4_op results
 */
void nfs4_op_openattr_Free(OPENATTR4res * resp)
{
  /* Nothing to be done */
  return;
} /* nfs4_op_openattr_Free */
