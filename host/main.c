/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <gp_socket_ta.h> 

static TEEC_Result open_session(TEEC_Context *ctx, TEEC_Session *sess)
{
        TEEC_UUID uuid = GP_SOCKET_API_TA_UUID;
        TEEC_Result res = TEEC_ERROR_GENERIC;
        uint32_t err_origin;

        /* Initialize a context connecting us to the TEE */
        res = TEEC_InitializeContext(NULL, ctx);
        if (res != TEEC_SUCCESS)
                errx(1, "NW: TEEC_InitializeContext failed with code 0x%x", res);

	/* Open the session with the Trusted Application */
        res = TEEC_OpenSession(ctx, sess, &uuid,
                               TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
        if (res != TEEC_SUCCESS)
                errx(1, "NW: TEEC_Opensession failed with code 0x%x origin 0x%x",
                        res, err_origin);

	return res;
}

static void close_session(TEEC_Context *ctx, TEEC_Session *sess)
{
        TEEC_CloseSession(sess);
        TEEC_FinalizeContext(ctx);
}


static TEEC_Result call_trusted_application(TEEC_Context *ctx,
					    TEEC_Session *sess,
					    uint32_t func_id)
{
	TEEC_Result res = TEEC_ERROR_GENERIC;
        TEEC_Operation op;
        uint32_t err_origin;
	uint32_t fd = 0;

        memset(&op, 0, sizeof(op));
        op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
                                         TEEC_NONE,
                                         TEEC_NONE,
                                         TEEC_NONE);

        op.params[0].value.a = 0xbabebabe;

	printf("NW: Invoke the TA (cmd: 0x%08x) with val: 0x%08x\n", func_id, fd);
        res = TEEC_InvokeCommand(sess, func_id, &op,
                                 &err_origin);
        if (res != TEEC_SUCCESS)
                errx(1, "NW: TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
                        res, err_origin);
        return res;
}  

int main() {
        TEEC_Context ctx;
        TEEC_Session sess;

	(void) open_session(&ctx, &sess);
	call_trusted_application(&ctx, &sess, GP_SOCKET_API_OPEN);
	(void) close_session(&ctx, &sess);

	return 0;
}
