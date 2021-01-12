#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <string.h>

#ifdef USE_SIGMA_XPU
#include <xrpclib.h>
#endif

#ifndef SUPPLIER_CERT_FILENAME
#define SUPPLIER_CERT_FILENAME		"supplier.cert.der"
#endif

#ifndef SUPPLIER_CA_CERT_FILENAME
#define SUPPLIER_CA_CERT_FILENAME		"/etc/up.cacert.der"
//#define SUPPLIER_CA_CERT_FILENAME		"up.cacert.der"
#endif

#define HASH_CHUNK_LENGTH		1024

#ifndef HASH_FILENAME
#define HASH_FILENAME			"/var/run/checksign.sha1"
#endif

#ifndef SV_LOG_LEVEL
#  define SV_LOG_LEVEL  7
#endif

#if SV_LOG_LEVEL > 0
#  define log(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#  define log(fmt, ...)  do { } while (0)
#endif

static int cb(int ok, X509_STORE_CTX *ctx);
#if SV_LOG_LEVEL > 0
static const char *usage = "Usage: ec_check (scratch#)  file2check\nExpects the signature named as file2check.sig and \nsupplier's public key certificate.\n";
#endif

int main(int argc, char *argv[]) {
    unsigned char *ecdsasig=0, *buf=0, *data=0, *cap=0, *scratch;
    char *filename=0;    
    int ecdsasiglen=0, ret=0, csize=0, ses=0, i;
    EVP_PKEY *supplier_pub_pkey=0;
    unsigned char cabuf[64*1024];
    FILE *fp;
    X509 *x509=0, *ca=0;
    X509_STORE *store=0;
    X509_STORE_CTX *verify_ctx=0;
    ECDSA_SIG *sig=0;
    BIO *in=NULL,*inp;
    BIO *bmd=NULL;
    const EVP_MD *md=NULL;


  if (argc == 3) {
  char *sign_file = NULL;
  sign_file = argv[2];
#ifdef USE_SIGMA_XPU
	scratch = getenv("XTASK_MARLIN_SCRATCH");
	if(scratch==NULL) { log("Read scratch failed\n"); ret=1; goto cleanup; }
	ses = atoi(scratch);
	if(ses < 0) { log("Invalid scratch number\n"); ret=2; goto cleanup; }
	printf("Opening scratch %d area\n",ses);
        if(xrpc_session_init(ses)<0) {log("Failed to open bus\n"); ret=3; goto cleanup;}
#endif
        
  // verification
  fp = fopen(SUPPLIER_CERT_FILENAME, "rb");  // read supplier cert
	if (fp == NULL) {log("Cannot load upgrade supplier's certificate\n");ret=4; goto cleanup;}
	fseek(fp,0,SEEK_END);
	csize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	data = (unsigned char *) calloc(1, csize);
	fread (data, csize, 1, fp);
	fclose(fp);			
  x509 = d2i_X509(NULL, (const unsigned char **)&data, csize);
  free(data-csize);
  csize=sizeof(cabuf);
	cap = cabuf;
  
#ifdef USE_SIGMA_XPU
	if(xpu_rpc_call(RPC_GETCA, (char*) cap, &csize, 0)) {
	    log("Problem with upgrade CA cert reading from secure flash\n"); 
	    ret=5; goto cleanup;
	}
#else
  FILE *f_ca = fopen(SUPPLIER_CA_CERT_FILENAME, "rb");  // read CA supplier cert
	if (f_ca == NULL) {log("Cannot load upgrade supplier's CA certificate %s\n",SUPPLIER_CA_CERT_FILENAME);ret=4; goto cleanup;}
	fseek(f_ca,0,SEEK_END);
	csize = ftell(f_ca);
	fseek(f_ca,0,SEEK_SET);
	fread (cap, csize, 1, f_ca);
	fclose(f_ca);			
#endif
  
	ca = d2i_X509(NULL, (const unsigned char **)&cap, csize);
        OpenSSL_add_all_algorithms();
        if (!(store=X509_STORE_new())) {log("Certificate verification error\n"); ret=6; goto cleanup;}
        X509_STORE_add_cert(store, ca);
	if(!(verify_ctx = X509_STORE_CTX_new())) {log("Certificate verification error\n"); ret=7; goto cleanup;}
	if(!(X509_STORE_CTX_init(verify_ctx, store, x509, NULL))) {log("Certificate verification error\n"); ret=8; goto cleanup;}
        X509_STORE_set_verify_cb_func(verify_ctx,cb);	
	if(!X509_verify_cert(verify_ctx)) { 
		log("Supplier certificate verification ERROR: %s\n", X509_verify_cert_error_string(verify_ctx->error)); 
		ret=9;
		goto cleanup;
	}
	log("Supplier certificate verified\n");
	data = (unsigned char *) calloc(1, HASH_CHUNK_LENGTH);
	if (data == NULL) {log("Error with memory allocation\n"); ret=10;goto cleanup;}
	md=EVP_sha1();
	in=BIO_new(BIO_s_file());
	bmd=BIO_new(BIO_f_md());
	if (!BIO_set_md(bmd,md)) {log("Error setting digest\n"); ret=11;goto cleanup;}
	inp=BIO_push(bmd,in);
	log("reading sign_file %s\n",sign_file);
  	BIO_read_filename(in,sign_file);
    
	for (;;) {
	    i=BIO_read(inp,(char *)data,HASH_CHUNK_LENGTH);
	    if(i < 0) { log("Read Error: %d\n",i); ret=12;	goto cleanup;}
	    if (i == 0) break;
	}

	BIO_gets(inp,(char *)data,HASH_CHUNK_LENGTH);
	
	if (NULL==(fp = fopen(HASH_FILENAME, "w"))) {log("Cannot open file to save hash\n"); ret=13;goto cleanup;}
	for(i=0;i<20;i++) fprintf(fp, "%02x", data[i]);
	fclose(fp);
	log("Created hash of %s, saved in %s\n", sign_file, HASH_FILENAME);
	filename = calloc(1, strlen(sign_file)+6);
	if (filename == NULL) {log("Error with memory allocation\n"); ret=14;goto cleanup;}
	strcpy(filename, sign_file);
	strncat(filename, ".sig", 4);
	fp = fopen(filename, "r");
	if (fp == NULL) {log("Cannot load the signature file"); ret=15; goto cleanup;}	
	fseek(fp,0,SEEK_END);
	ecdsasiglen = ftell(fp);
	fseek(fp,0,SEEK_SET);
	buf = OPENSSL_malloc(ecdsasiglen);
	ecdsasig = buf;
	fread (ecdsasig, ecdsasiglen, 1, fp);
	fclose(fp);
	sig = ECDSA_SIG_new();
	sig=d2i_ECDSA_SIG(&sig,(const unsigned char **)&ecdsasig,ecdsasiglen);
	if(sig == NULL) {log("transformation error\n"); ret=16; goto cleanup;}
	supplier_pub_pkey=X509_get_pubkey(x509);
	if(supplier_pub_pkey == NULL) {log("supplier public key error\n"); ret=16; goto cleanup;}
	if(!ECDSA_do_verify(data, 20, sig, supplier_pub_pkey->pkey.ec)) { 
	    log("Signature verify FAILED\n");
	    ret=17; goto cleanup;
	}
	log("Signature verified OK\n");
   } else {
#if SV_LOG_LEVEL > 0
        fprintf(stderr,"%s", usage);
#endif
        exit(1);
   }
cleanup:
	if (data) free(data);
	if (buf) OPENSSL_free(buf);
	if (sig) ECDSA_SIG_free(sig);
	if (supplier_pub_pkey) EVP_PKEY_free (supplier_pub_pkey);
	if (verify_ctx) X509_STORE_CTX_free(verify_ctx);
	if (filename) free(filename);
	if (x509) X509_free(x509);
        if (ca) X509_free(ca);
        if (store) X509_STORE_free(store);
#ifdef USE_SIGMA_XPU
	xrpc_session_destroy();
#endif
        return ret;
}

static int cb(int ok, X509_STORE_CTX *ctx) {
    if (!ok) {
        if (ctx->error == X509_V_ERR_CERT_HAS_EXPIRED) ok=1;
        if (ctx->error == X509_V_ERR_CERT_NOT_YET_VALID) ok=1;
    }
    return (ok);
}
