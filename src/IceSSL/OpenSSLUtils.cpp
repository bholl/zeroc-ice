// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StaticMutex.h>
#include <IceSSL/OpenSSLPluginI.h>
#include <IceSSL/OpenSSLUtils.h>

#include <openssl/err.h>

using namespace std;

// The following arrays are compiled-in diffie hellman group parameters.
// These are used when OpenSSL opts to use ephemeral diffie-hellman keys
// and no group parameters have been supplied in the SSL configuration
// files.  These are known strong primes, distributed with the OpenSSL
// library in the files dh512.pem, dh1024.pem, dh2048.pem and dh4096.pem.
// They are not keys themselves, but the basis for generating DH keys
// on the fly.

static unsigned char dh512_p[] =
{
    0xF5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
    0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
    0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
    0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
    0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
    0xE9,0x2A,0x05,0x5F,
};

static unsigned char dh512_g[] = { 0x02, };

static unsigned char dh1024_p[] =
{
    0xF4,0x88,0xFD,0x58,0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,
    0x91,0x07,0x36,0x6B,0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,
    0x88,0xB3,0x1C,0x7C,0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,
    0x43,0xF0,0xA5,0x5B,0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,
    0x38,0xD3,0x34,0xFD,0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,
    0xDE,0x33,0x21,0x2C,0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,
    0x18,0x11,0x8D,0x7C,0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,
    0x19,0xC8,0x07,0x29,0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,
    0xD0,0x0A,0x50,0x9B,0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,
    0x41,0x9F,0x9C,0x7C,0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,
    0xA2,0x5E,0xC3,0x55,0xE9,0x2F,0x78,0xC7,
};

static unsigned char dh1024_g[] = { 0x02, };

static unsigned char dh2048_p[] =
{
    0xF6,0x42,0x57,0xB7,0x08,0x7F,0x08,0x17,0x72,0xA2,0xBA,0xD6,
    0xA9,0x42,0xF3,0x05,0xE8,0xF9,0x53,0x11,0x39,0x4F,0xB6,0xF1,
    0x6E,0xB9,0x4B,0x38,0x20,0xDA,0x01,0xA7,0x56,0xA3,0x14,0xE9,
    0x8F,0x40,0x55,0xF3,0xD0,0x07,0xC6,0xCB,0x43,0xA9,0x94,0xAD,
    0xF7,0x4C,0x64,0x86,0x49,0xF8,0x0C,0x83,0xBD,0x65,0xE9,0x17,
    0xD4,0xA1,0xD3,0x50,0xF8,0xF5,0x59,0x5F,0xDC,0x76,0x52,0x4F,
    0x3D,0x3D,0x8D,0xDB,0xCE,0x99,0xE1,0x57,0x92,0x59,0xCD,0xFD,
    0xB8,0xAE,0x74,0x4F,0xC5,0xFC,0x76,0xBC,0x83,0xC5,0x47,0x30,
    0x61,0xCE,0x7C,0xC9,0x66,0xFF,0x15,0xF9,0xBB,0xFD,0x91,0x5E,
    0xC7,0x01,0xAA,0xD3,0x5B,0x9E,0x8D,0xA0,0xA5,0x72,0x3A,0xD4,
    0x1A,0xF0,0xBF,0x46,0x00,0x58,0x2B,0xE5,0xF4,0x88,0xFD,0x58,
    0x4E,0x49,0xDB,0xCD,0x20,0xB4,0x9D,0xE4,0x91,0x07,0x36,0x6B,
    0x33,0x6C,0x38,0x0D,0x45,0x1D,0x0F,0x7C,0x88,0xB3,0x1C,0x7C,
    0x5B,0x2D,0x8E,0xF6,0xF3,0xC9,0x23,0xC0,0x43,0xF0,0xA5,0x5B,
    0x18,0x8D,0x8E,0xBB,0x55,0x8C,0xB8,0x5D,0x38,0xD3,0x34,0xFD,
    0x7C,0x17,0x57,0x43,0xA3,0x1D,0x18,0x6C,0xDE,0x33,0x21,0x2C,
    0xB5,0x2A,0xFF,0x3C,0xE1,0xB1,0x29,0x40,0x18,0x11,0x8D,0x7C,
    0x84,0xA7,0x0A,0x72,0xD6,0x86,0xC4,0x03,0x19,0xC8,0x07,0x29,
    0x7A,0xCA,0x95,0x0C,0xD9,0x96,0x9F,0xAB,0xD0,0x0A,0x50,0x9B,
    0x02,0x46,0xD3,0x08,0x3D,0x66,0xA4,0x5D,0x41,0x9F,0x9C,0x7C,
    0xBD,0x89,0x4B,0x22,0x19,0x26,0xBA,0xAB,0xA2,0x5E,0xC3,0x55,
    0xE9,0x32,0x0B,0x3B,
};

static unsigned char dh2048_g[] = { 0x02, };

static unsigned char dh4096_p[] =
{
    0xFA,0x14,0x72,0x52,0xC1,0x4D,0xE1,0x5A,0x49,0xD4,0xEF,0x09,
    0x2D,0xC0,0xA8,0xFD,0x55,0xAB,0xD7,0xD9,0x37,0x04,0x28,0x09,
    0xE2,0xE9,0x3E,0x77,0xE2,0xA1,0x7A,0x18,0xDD,0x46,0xA3,0x43,
    0x37,0x23,0x90,0x97,0xF3,0x0E,0xC9,0x03,0x50,0x7D,0x65,0xCF,
    0x78,0x62,0xA6,0x3A,0x62,0x22,0x83,0xA1,0x2F,0xFE,0x79,0xBA,
    0x35,0xFF,0x59,0xD8,0x1D,0x61,0xDD,0x1E,0x21,0x13,0x17,0xFE,
    0xCD,0x38,0x87,0x9E,0xF5,0x4F,0x79,0x10,0x61,0x8D,0xD4,0x22,
    0xF3,0x5A,0xED,0x5D,0xEA,0x21,0xE9,0x33,0x6B,0x48,0x12,0x0A,
    0x20,0x77,0xD4,0x25,0x60,0x61,0xDE,0xF6,0xB4,0x4F,0x1C,0x63,
    0x40,0x8B,0x3A,0x21,0x93,0x8B,0x79,0x53,0x51,0x2C,0xCA,0xB3,
    0x7B,0x29,0x56,0xA8,0xC7,0xF8,0xF4,0x7B,0x08,0x5E,0xA6,0xDC,
    0xA2,0x45,0x12,0x56,0xDD,0x41,0x92,0xF2,0xDD,0x5B,0x8F,0x23,
    0xF0,0xF3,0xEF,0xE4,0x3B,0x0A,0x44,0xDD,0xED,0x96,0x84,0xF1,
    0xA8,0x32,0x46,0xA3,0xDB,0x4A,0xBE,0x3D,0x45,0xBA,0x4E,0xF8,
    0x03,0xE5,0xDD,0x6B,0x59,0x0D,0x84,0x1E,0xCA,0x16,0x5A,0x8C,
    0xC8,0xDF,0x7C,0x54,0x44,0xC4,0x27,0xA7,0x3B,0x2A,0x97,0xCE,
    0xA3,0x7D,0x26,0x9C,0xAD,0xF4,0xC2,0xAC,0x37,0x4B,0xC3,0xAD,
    0x68,0x84,0x7F,0x99,0xA6,0x17,0xEF,0x6B,0x46,0x3A,0x7A,0x36,
    0x7A,0x11,0x43,0x92,0xAD,0xE9,0x9C,0xFB,0x44,0x6C,0x3D,0x82,
    0x49,0xCC,0x5C,0x6A,0x52,0x42,0xF8,0x42,0xFB,0x44,0xF9,0x39,
    0x73,0xFB,0x60,0x79,0x3B,0xC2,0x9E,0x0B,0xDC,0xD4,0xA6,0x67,
    0xF7,0x66,0x3F,0xFC,0x42,0x3B,0x1B,0xDB,0x4F,0x66,0xDC,0xA5,
    0x8F,0x66,0xF9,0xEA,0xC1,0xED,0x31,0xFB,0x48,0xA1,0x82,0x7D,
    0xF8,0xE0,0xCC,0xB1,0xC7,0x03,0xE4,0xF8,0xB3,0xFE,0xB7,0xA3,
    0x13,0x73,0xA6,0x7B,0xC1,0x0E,0x39,0xC7,0x94,0x48,0x26,0x00,
    0x85,0x79,0xFC,0x6F,0x7A,0xAF,0xC5,0x52,0x35,0x75,0xD7,0x75,
    0xA4,0x40,0xFA,0x14,0x74,0x61,0x16,0xF2,0xEB,0x67,0x11,0x6F,
    0x04,0x43,0x3D,0x11,0x14,0x4C,0xA7,0x94,0x2A,0x39,0xA1,0xC9,
    0x90,0xCF,0x83,0xC6,0xFF,0x02,0x8F,0xA3,0x2A,0xAC,0x26,0xDF,
    0x0B,0x8B,0xBE,0x64,0x4A,0xF1,0xA1,0xDC,0xEE,0xBA,0xC8,0x03,
    0x82,0xF6,0x62,0x2C,0x5D,0xB6,0xBB,0x13,0x19,0x6E,0x86,0xC5,
    0x5B,0x2B,0x5E,0x3A,0xF3,0xB3,0x28,0x6B,0x70,0x71,0x3A,0x8E,
    0xFF,0x5C,0x15,0xE6,0x02,0xA4,0xCE,0xED,0x59,0x56,0xCC,0x15,
    0x51,0x07,0x79,0x1A,0x0F,0x25,0x26,0x27,0x30,0xA9,0x15,0xB2,
    0xC8,0xD4,0x5C,0xCC,0x30,0xE8,0x1B,0xD8,0xD5,0x0F,0x19,0xA8,
    0x80,0xA4,0xC7,0x01,0xAA,0x8B,0xBA,0x53,0xBB,0x47,0xC2,0x1F,
    0x6B,0x54,0xB0,0x17,0x60,0xED,0x79,0x21,0x95,0xB6,0x05,0x84,
    0x37,0xC8,0x03,0xA4,0xDD,0xD1,0x06,0x69,0x8F,0x4C,0x39,0xE0,
    0xC8,0x5D,0x83,0x1D,0xBE,0x6A,0x9A,0x99,0xF3,0x9F,0x0B,0x45,
    0x29,0xD4,0xCB,0x29,0x66,0xEE,0x1E,0x7E,0x3D,0xD7,0x13,0x4E,
    0xDB,0x90,0x90,0x58,0xCB,0x5E,0x9B,0xCD,0x2E,0x2B,0x0F,0xA9,
    0x4E,0x78,0xAC,0x05,0x11,0x7F,0xE3,0x9E,0x27,0xD4,0x99,0xE1,
    0xB9,0xBD,0x78,0xE1,0x84,0x41,0xA0,0xDF,
};

static unsigned char dh4096_g[] = { 0x02, };

// Ensures that the sslGetErrors() function is synchronized.
static IceUtil::StaticMutex sslErrorsMutex = ICE_STATIC_MUTEX_INITIALIZER;

//
// NOTE: The following (mon, getGeneralizedTime, getUTCTime and getASN1time)
//       are routines that have been abducted from the OpenSSL X509 library,
//       and modified to work with the STL basic_string template.

static const char* mon[12]=
{
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

string
IceSSL::getGeneralizedTime(ASN1_GENERALIZEDTIME *tm)
{
    assert(tm != 0);

    char buf[30];
    int gmt = 0, y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

    int i = tm->length;

    char* v = (char *) tm->data;

    if(i < 12)
    {
        goto err;
    }

    if(v[i-1] == 'Z')
    {
        gmt=1;
    }

    for(i=0; i<12; i++)
    {
        if((v[i] > '9') || (v[i] < '0'))
        {
            goto err;
        }
    }

    y = (v[0] - '0') * 1000 + (v[1] - '0') * 100 + (v[2] - '0') * 10 + (v[3] - '0');
    M = (v[4] - '0') * 10 + (v[5] - '0');

    if((M > 12) || (M < 1))
    {
        goto err;
    }

    d = (v[6] - '0') * 10 + (v[7] - '0');
    h = (v[8] - '0') * 10 + (v[9] - '0');
    m = (v[10] - '0') * 10 + (v[11] - '0');

    if((v[12] >= '0') && (v[12] <= '9') &&
        (v[13] >= '0') && (v[13] <= '9'))
    {
        s = (v[12] - '0') * 10 + (v[13] - '0');
    }

    sprintf(buf, "%s %2d %02d:%02d:%02d %d%s", mon[M-1], d, h, m, s, y, (gmt)?" GMT":"");
    return string(buf);

err:
    return string("Bad time value");
}

string
IceSSL::getUTCTime(ASN1_UTCTIME *tm)
{
    assert(tm != 0);

    char buf[30];
    int gmt = 0, y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

    int i = tm->length;
    char* v = (char *) tm->data;

    if(i < 10)
    { 
        goto err;
    }

    if(v[i-1] == 'Z')
    {
        gmt=1;
    }

    for(i = 0; i < 10; i++)
    {
        if((v[i] > '9') || (v[i] < '0'))
        {
            goto err;
        }
    }

    y = (v[0] - '0') * 10 + (v[1] - '0');

    if(y < 50)
    {
        y+=100;
    }

    M = (v[2] - '0') * 10 + (v[3] - '0');

    if((M > 12) || (M < 1))
    {
        goto err;
    }

    d = (v[4] - '0') * 10 + (v[5] - '0');
    h = (v[6] - '0') * 10 + (v[7] - '0');
    m = (v[8] - '0') * 10 + (v[9] - '0');

    if((v[10] >= '0') && (v[10] <= '9') && (v[11] >= '0') && (v[11] <= '9'))
    {
        s = (v[10] - '0') * 10 + (v[11] - '0');
    }

    sprintf(buf, "%s %2d %02d:%02d:%02d %d%s", mon[M-1], d, h, m, s, y+1900, (gmt)?" GMT":"");
    return string(buf);

err:
    return string("Bad time value");
}

string
IceSSL::getASN1time(ASN1_TIME *tm)
{
    assert(tm != 0);

    string theTime;

    switch(tm->type)
    {
        case V_ASN1_UTCTIME :
        {
            theTime = getUTCTime(tm);
            break;
        }

        case V_ASN1_GENERALIZEDTIME :
        {
	    theTime = getGeneralizedTime(tm);
            break;
        }

        default :
        {
            theTime = "Bad time value";
            break;
        }
    }

    return theTime;
}

DH*
IceSSL::loadDHParam(const char* dhfile)
{
    assert(dhfile != 0);

    DH* ret = 0;
    BIO* bio = BIO_new_file(dhfile,"r");

    if(bio != 0)
    {
        ret = PEM_read_bio_DHparams(bio, 0, 0, 0);
        BIO_free(bio);
    }

    return ret;
}

DH*
IceSSL::getTempDH(unsigned char* p, int plen, unsigned char* g, int glen)
{
    assert(p != 0);
    assert(g != 0);

    DH* dh = DH_new();

    if(dh != 0)
    {
        dh->p = BN_bin2bn(p, plen, 0);

        dh->g = BN_bin2bn(g, glen, 0);

        if((dh->p == 0) || (dh->g == 0))
        {
            // Note: Clears both p and g if they are not NULL.
            DH_free(dh);
            dh = 0;
        }
    }

    return dh;
}

DH*
IceSSL::getTempDH512()
{
    return getTempDH(dh512_p, (int) sizeof(dh512_p), dh512_g, (int) sizeof(dh512_g));
}

DH*
IceSSL::getTempDH1024()
{
    return getTempDH(dh1024_p, (int) sizeof(dh1024_p), dh1024_g, (int) sizeof(dh1024_g));
}

DH*
IceSSL::getTempDH2048()
{
    return getTempDH(dh2048_p, (int) sizeof(dh2048_p), dh2048_g, (int) sizeof(dh2048_g));
}

DH*
IceSSL::getTempDH4096()
{
    return getTempDH(dh4096_p, (int) sizeof(dh4096_p), dh4096_g, (int) sizeof(dh4096_g));
}

string
IceSSL::sslGetErrors()
{
    IceUtil::StaticMutex::Lock sync(sslErrorsMutex);
    
    string errorMessage;
    char buf[200];
    char bigBuffer[1024];
    const char* file = 0;
    const char* data = 0;
    int line = 0;
    int flags = 0;
    unsigned long errorCode = 0;
    int errorNum = 1;

    unsigned long es = CRYPTO_thread_id();

    while((errorCode = ERR_get_error_line_data(&file, &line, &data, &flags)) != 0)
    {
        sprintf(bigBuffer,"%6d - Thread ID: %lu\n", errorNum, es);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Error:     %lu\n", errorNum, errorCode);
        errorMessage += bigBuffer;

        // Request an error from the OpenSSL library
        ERR_error_string_n(errorCode, buf, sizeof(buf));
        sprintf(bigBuffer,"%6d - Message:   %s\n", errorNum, buf);
        errorMessage += bigBuffer;

        sprintf(bigBuffer,"%6d - Location:  %s, %d\n", errorNum, file, line);
        errorMessage += bigBuffer;

        if(flags & ERR_TXT_STRING)
        {
            sprintf(bigBuffer,"%6d - Data:      %s\n", errorNum, data);
            errorMessage += bigBuffer;
        }

        errorNum++;
    }

    ERR_clear_error();

    return errorMessage;
}

static const char* errorStrings[] =
{
    "Unable to get issuer's certificate.",
    "Unable to get certificate revocation list.",
    "Unable to decrypt certificate signature.",
    "Unable to decrypt certificate revocation list signature.",
    "Unable to decode issuer's public key.",
    "Certificate signature failure.",
    "Certificate revocation list signature failure.",
    "Certificate not yet valid.",
    "Certificate has expired.",
    "Certificate revocation list not yet valid.",
    "Certificate revocation list has expired.",
    "Error in certificate's \"not before\" field",
    "Error in certificate's \"not after\" field",
    "Error in the certificate revocation list's \"last update\" field",
    "Error in the certificate revocation list's \"next update\" field",
    "Out of memory failure.",
    "Encountered a zero-depth self-signed certificate.",
    "Encountered self-signed certificate in the certificate chain.",
    "Unable to get issuer certificate locally.",
    "Unable to verify leaf signature.",
    "Certificate chain too long.",
    "Certificate has been revoked.",
    "Invalid certificate authority.",
    "Certificate Authority path length exceeded.",
    "Invalid certificate purpose.",
    "Certificate is untrusted.",
    "Certificate is rejected.",
    "Subject and Issuer do not match.",
    "AKID/SKID mismatch.",
    "AKID and Issuer Serial mismatch.",
    "Key usage precludes certifiicate signing.",
    "Application verification."
};

string
IceSSL::getVerificationError(long errorCode)
{
    string errString;

    if(errorCode > X509_V_ERR_KEYUSAGE_NO_CERTSIGN)
    {
        if(errorCode == X509_V_ERR_APPLICATION_VERIFICATION)
        {
	    errString = "Application Verification error.";
        }
	else
        {
            ostringstream errStream;
	    errStream << "Unknown error code: " << dec << errorCode << "."; 
	    errString = errStream.str();
        }
    }
    else
    {
        errorCode -= 2;
	errString = errorStrings[errorCode];
    }

    return errString;
}

extern "C"
{

RSA*
tmpRSACallback(SSL* sslConnection, int isExport, int keyLength)
{
    assert(sslConnection != 0);

    void* p = SSL_get_ex_data(sslConnection, 0);
    assert(p != 0);
    IceSSL::OpenSSLPluginI* openSslPlugin = static_cast<IceSSL::OpenSSLPluginI*>(p);
    assert(openSslPlugin != 0);

    return openSslPlugin->getRSAKey(isExport, keyLength);
}

DH*
tmpDHCallback(SSL* sslConnection, int isExport, int keyLength)
{
    assert(sslConnection != 0);

    void* p = SSL_get_ex_data(sslConnection, 0);
    assert(p != 0);
    IceSSL::OpenSSLPluginI* openSslPlugin = static_cast<IceSSL::OpenSSLPluginI*>(p);

    assert(openSslPlugin != 0);

    return openSslPlugin->getDHParams(isExport, keyLength);
}

// verifyCallback - Certificate Verification callback function.
int
verifyCallback(int ok, X509_STORE_CTX* ctx)
{
    assert(ctx != 0);

    // Tricky method to get access to our connection.  I would use
    // SSL_get_ex_data() to get the Connection object, if only I had
    // some way to retrieve the index of the object in this function.
    // Hence, we have to invent our own reference system here.
    SSL* sslConnection = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
    assert(sslConnection != 0);

    IceSSL::SslTransceiverPtr transceiver = IceSSL::SslTransceiver::getTransceiver(sslConnection);
    assert(transceiver != 0);

    // Call the connection, get it to perform the verification.
    return transceiver->verifyCertificate(ok, ctx);
}

}
