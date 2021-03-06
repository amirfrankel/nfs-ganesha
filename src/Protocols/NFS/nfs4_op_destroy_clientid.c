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
 * @file nfs4_op_destroy_clientid.c
 * @brief Provides NFS4_OP_DESTROY_CLIENTID implementation
 */

#include "config.h"
#include <pthread.h>
#include "log.h"
#include "nfs4.h"
#include "sal_functions.h"
#include "nfs_proto_functions.h"
#include "nfs_core.h"

/**
 *
 * @brief The NFS4_OP_DESTROY_CLIENTID operation.
 *
 * @param[in]     op   nfs4_op arguments
 * @param[in,out] data Compound request data
 * @param[out]    resp nfs4_op results
 *
 * @retval NFS4_OK or errors for NFSv4.1.
 * @retval NFS4ERR_NOTSUPP for NFSv4.0.
 *
 */

int nfs4_op_destroy_clientid(struct nfs_argop4 *op,
                             compound_data_t   *data,
                             struct nfs_resop4 *resp)
{
	nfs_client_record_t *client_record = NULL;
	nfs_client_id_t *conf = NULL, *unconf = NULL, *found;
        clientid4 clientid;
	int rc;

#define arg_DESTROY_CLIENTID4 op->nfs_argop4_u.opdestroy_clientid
#define res_DESTROY_CLIENTID4 resp->nfs_resop4_u.opdestroy_clientid

	resp->resop = NFS4_OP_DESTROY_CLIENTID;
	if (data->minorversion < 1) {
		res_DESTROY_CLIENTID4.dcr_status = NFS4ERR_NOTSUPP;
		return res_DESTROY_CLIENTID4.dcr_status;
	}

        clientid = arg_DESTROY_CLIENTID4.dca_clientid;

        LogDebug(COMPONENT_CLIENTID,
                 "DESTROY_CLIENTID clientid=%"PRIx64, clientid);

        res_DESTROY_CLIENTID4.dcr_status = NFS4_OK;

	/* First try to look up confirmed record */
	rc = nfs_client_id_get_confirmed(clientid, &conf);
        if (rc == CLIENT_ID_SUCCESS) {
            client_record = conf->cid_client_record;
            found = conf;
        } else {
            /* fall back to unconfirmed */
            rc = nfs_client_id_get_unconfirmed(clientid, &unconf);
            if (rc == CLIENT_ID_SUCCESS) {
                client_record = unconf->cid_client_record;
                found = unconf;
            }
            /* handle the perverse case of a clientid being confirmed
             * in the above interval */
            rc = nfs_client_id_get_confirmed(clientid, &conf);
            if (rc == CLIENT_ID_SUCCESS) {
                client_record = conf->cid_client_record;
                found = conf;
            }
        }

        /* ref +1 */
	if (client_record == NULL) {
            /* Fine.  We're done. */
            goto out;
	}

	P(client_record->cr_mutex);

	if (isFullDebug(COMPONENT_CLIENTID)) {
		char str[HASHTABLE_DISPLAY_STRLEN];

		display_client_record(client_record, str);

		LogFullDebug(COMPONENT_CLIENTID,
			     "Client Record %s cr_confirmed_rec=%p "
			     "cr_unconfirmed_rec=%p",
			     str,
			     client_record->cr_confirmed_rec,
			     client_record->cr_unconfirmed_rec);
	}

        /* per Frank, we must check the confirmed and unconfirmed
         * state of client_record again now that we hold cr_mutex
         */
        conf = client_record->cr_confirmed_rec;
        unconf = client_record->cr_unconfirmed_rec;
        if ((! conf) && (! unconf)) {
            /* We raced a thread destroying clientid, and lost.
             * We're done. */
            goto cleanup;
        }

        /* We MUST NOT destroy a clientid that has nfsv41 sessions or state.
         * Since the minorversion is 4.1 or higher, this is equivalent to a
         * session check. */
        if (client_id_has_nfs41_sessions(found)) {
                res_DESTROY_CLIENTID4.dcr_status = NFS4ERR_CLID_INUSE;
                goto cleanup;
        }

	if (conf) {
            /* Delete the confirmed clientid record. Because we
             * have the cr_mutex, we have won any race to deal
             * with this clientid record.
             */
            if (isFullDebug(COMPONENT_CLIENTID)) {
                char str[HASHTABLE_DISPLAY_STRLEN];

                display_client_id_rec(conf, str);

                LogDebug(COMPONENT_CLIENTID,
                         "Removing confirmed clientid %s",
                         str);
            }

            /* unhash the clientid record */
            (void) remove_confirmed_client_id(conf);
            conf = NULL;
        }

	if (unconf) {
		/* Delete the unconfirmed clientid record. Because we
		 * have the cr_mutex, we have won any race to deal
		 * with this clientid record.
		 */
		if (isFullDebug(COMPONENT_CLIENTID)) {
			char str[HASHTABLE_DISPLAY_STRLEN];

			display_client_id_rec(unconf, str);

			LogDebug(COMPONENT_CLIENTID,
				 "Removing unconfirmed clientid %s",
				 str);
		}

		/* unhash the clientid record */
		(void) remove_unconfirmed_client_id(unconf);
		unconf = NULL;
	}

cleanup:
        if (client_record) {
            V(client_record->cr_mutex);
            (void) dec_client_record_ref(client_record); /* ref +0 */
        }

out:
	return (res_DESTROY_CLIENTID4.dcr_status);
}

/**
 * @brief Free DESTROY_CLIENTID result
 *
 * @param[in,out] resp nfs4_op results
 */

void nfs4_op_destroyclientid_Free(SETCLIENTID4res *resp)
{
	return;
}
