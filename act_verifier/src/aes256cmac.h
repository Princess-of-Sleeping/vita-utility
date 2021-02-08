/*
 * PSVita act verifier
 * Copyright (C) 2021, Princess of Sleeping
 */

#ifndef _ACT_VERIFIER_AES256CMAC_H_
#define _ACT_VERIFIER_AES256CMAC_H_

void actVerifierAes256cmac(const void *key, const void *src, SceSize length, void *cmac);

#endif /* _ACT_VERIFIER_AES256CMAC_H_ */
