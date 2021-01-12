#include <stdio.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/x509.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>

#define CONFIG_FILE "/opt/secure-upgrade-tools/signtool/conf/sign_conf"

void print_help() {
   printf("\nUsage: sign [-h] [-k key] [-c certificate] [-f config_file] -i input_file\n\n");
   printf("-i input_file     - specify file to sign.\n");
   printf("-k key            - specify key file use to sign.\n");
   printf("-c certificate    - specify certificate of the signing firmware.\n");
   printf("-h                - see help.\n\n");
}

int file_exist(char *filename){
   FILE *fp;
   fp = fopen(filename, "r");
   if(fp){
      fclose(fp);
      return 1;
   }
   else{
      return 0;
   }
}

int parse_line(char *line,char *variable, char **var_value)
{
   if (!line)
      return 0;

   char name[128], value[512];
   char *pname=name;
   char *pvalue=value;
   memset(name, 0, sizeof(name));
   memset(value, 0, sizeof(value));
   char separator = '=';
   if ((*line == '#') || (*line == '='))
      return 0;
   while ( (*line != separator) && (*line != '\0') )
   {
      if (*line == '\n')
         return 0;

      //tolower
      if ( (*line >= 'A') && (*line <= 'Z'))
         *line+= ('a'-'A');
      if( (*line == '\"') || (*line == '\'') || (*line == '\t') || (*line == ' ') )
      {
         line++;
         continue;
      }
      *pname = *line;
      line++;
      pname++;
   }

   if( ! strcmp(name,variable) ){
      while ( *line != '\0')
      {
         if( (*line == '\"') || (*line == '\'') || (*line == '\t') || (*line == ' ') || (*line == '\n') || (*line == separator))
         {
            line++;
            continue;
         }
         *pvalue = *line;
         line++;
         pvalue++;
      }
      *var_value = strdup(value);
      return 1;
   }
   else{
      return 0;
   }
}

char* basename(char* path){
   if(! path) return 0;
   int len = strlen(path);
   char *ptr=path;
   int i,last_sl=-1;
   for(i=0;i<len;i++){
      if(*ptr == '/')
         last_sl = i;
      ptr++;
   }
   char* ret = calloc(1,len - last_sl);
   memcpy(ret,path+last_sl+1,len-last_sl-1);
   return ret;
}

char* dirname(char* in){
   if(! in) return 0;
   int len = strlen(in);
   char *ptr=in;
   int i, last=-1;
   for(i=0;i<len;i++){
      if(*ptr == '/')
         last = i;
      ptr++;
   }
   if(last == -1) return 0;
   char *ret =  calloc(1,last+1);
   memcpy(ret,in,last);
   return ret;
}




int main(int argc, char *argv[]) {

   char *key_file=0, *input_file=0, *certificate_file=0, *f_name=0;
   FILE *fp;

   static struct option longopts[] = {
      {"help", 0, 0, 'h'},
      {"input", 1, 0, 'i'},
      {"output", 1, 0, 'o'},
      {"key",1,0,'k'},
      {"certificate",1,0,'c'},
      {0, 0, 0, 0}
   };
   int opt;
   while ((opt = getopt_long(argc, argv, "k:i:o:c:f:h", longopts, NULL)) != -1)
   {
      switch (opt)
      {
         case 'h':
            print_help();
            return 0;
         case 'k':
            key_file = strdup(optarg);
            break;
         case 'i':
            input_file = strdup(optarg);
            break;
         case 'c':
            certificate_file = strdup(optarg);
            break;
         default:
            break;
      }
   }

   //////////////Checking parametrs//////////////////////////////
   //input file
   if( input_file ){
      if( ! file_exist(input_file) ){
         printf("Can not open %s file \n ",input_file);
         return 1;
      }
   }
   else{
      printf("Input file not specify. Use -h option to see usage\n");
      return 1;
   }

   //key file
   if( key_file ){
      if( ! file_exist(key_file) ){
         printf("Can not open %s file \n",key_file);
         return 1;
      }
   }
   else{
      printf("Can not find key to sign - private key. Specify it using -k option.\n");
   }

   //certificate
   if( certificate_file ){
      if( ! file_exist(certificate_file) ) {
         printf("Can not open %s file\n",certificate_file);
         return 1;
      }
   }
   else{
      printf("Can not find certificate - public key. Specify it using -c option.\n");
   }



   /////////////////////////signing
   unsigned char *ecdsasig=0, *buf=0, *data=0;
   int ecdsasiglen, ssize;
   EVP_PKEY *server_pkey;
   unsigned char md[20];
   ECDSA_SIG *sig;

   fp = fopen(input_file, "r");
   fseek(fp,0,SEEK_END);
   ssize = ftell(fp);
   fseek(fp,0,SEEK_SET);
   data = (unsigned char *) calloc(1, ssize);
   fread(data, ssize, 1, fp);
   fclose(fp);
   SHA1(data, ssize, md);
   free(data);
   fp = fopen(key_file, "r");
   fseek(fp,0,SEEK_END);
   ssize = ftell(fp);
   fseek(fp,0,SEEK_SET);
   data = (unsigned char *) calloc(1, ssize);
   fread (data, ssize, 1, fp);
   fclose(fp);
   server_pkey = d2i_PrivateKey(EVP_PKEY_EC,NULL,(void *)&data,ssize);
   free(data-ssize);
   sig = ECDSA_do_sign(md, 20, EVP_PKEY_get1_EC_KEY(server_pkey));
   EVP_PKEY_free (server_pkey);
   if(!sig) { printf("ERROR sign\n"); return 1;}
   ecdsasiglen = i2d_ECDSA_SIG(sig,NULL);
   buf = OPENSSL_malloc(ecdsasiglen);
   ecdsasig = buf;
   ecdsasiglen = i2d_ECDSA_SIG(sig,&ecdsasig);
   ECDSA_SIG_free(sig);

   char *signature=0;
   f_name = basename(input_file);
   int s_len = 6;
   if(f_name) s_len+=strlen(f_name);
   signature = calloc(1,s_len);
   strcat(signature,f_name);
   strcat(signature,".sig");
   fp = fopen(signature, "wb");
   fwrite(ecdsasig-ecdsasiglen, sizeof(ecdsasig[0]), ecdsasiglen, fp);
   fclose(fp);
  // printf("Created signature of %s \nsaved in %s\n", input_file, signature);
   OPENSSL_free(buf);

   free(input_file);
   free(key_file);
   free(certificate_file);
   free(signature);

   return 0;
}

