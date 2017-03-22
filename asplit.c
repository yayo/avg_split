/* asplit.c -- split a file into average sized pieces.
   Copyright (C) 1988-2017 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/* By: "Robert Young" <yayooo@gmail.com>  */
/*

gcc -Wall -Wextra -Werror asplit.c -lm -o asplit

# size(FILE_(N)) < size(FILE_(1))
# size(FILE_(N-1)) > size(FILE_(N-2))
# size(FILE_(n+1))-size(FILE_(n))=1 n∈[1,N-2]

# Inspecting the available pieces you may deduce the missing pieces.

Known Answer Test:
L=9 ; for S in {1..45} ; do dd count=1 if=/dev/urandom bs=${S} of=asplit.test.${S} 2>/dev/null ; ./asplit asplit.test.${S} ${L} ; wc -c asplit.test.${S}.* > kat_${L}/asplit.test.${S}.answer ; rm asplit.test.${S}* ; done

# size(27) limit(9) : 7 8 9 3
# size(1048567) limit(131075) : 131069 131070 131071 131072 131073 131074 131075 131063

*/

#include <stdio.h>
#define __USE_GNU
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include <math.h>
#include <assert.h>
#include <libgen.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>

int out(char *v)
 {int o;
  if(-1==(o=open(v,O_CREAT|O_EXCL|O_WRONLY,S_IRUSR|S_IWUSR)))
   {fprintf(stderr,"Create output piece failed: %s\n",v);
   }
  return(o);
 }

int main(int argc, char *argv[])
 {if(3>argc||6<argc)
   {fprintf(stderr,
"Usage: %s Input_File_to_Split Limit_Size Suffix(.) Output_Dir(for_Splited_Files) Prefix(for_Splited_Files)\n\n\
Split file into almost average sized pieces with following warranties :\n\
1. Different pieces have DIFFERENT size\n\
2. Produces LESS pieces as possible\n\
3. Each pieces have a SIMILAR size\n\
4. The penultimate piece has the MAX size\n\
5. The last piece has the MIN size\n\
6. From the first piece to the penultimate piece obey a size increment of 1\n\
7. first-N<=last<=first-1\n\
",argv[0]);
    return(-1);
   }
  else
   {const char *x=".";
    if(4<=argc) x=argv[3];
    size_t i=strlen(x);
    const char g[]="%01d";
    char a[i+sizeof(g)];
    snprintf(a,sizeof(a),"%s%s",x,g);
    const char *d=(5<=argc?argv[4]:dirname(strdupa(argv[1])));
    const char *b=(6==argc?argv[5]:basename(strdupa(argv[1])));
    const size_t t=sizeof(a)+9;
    size_t e=strlen(d)+1+strlen(b);
    char *v=alloca(e+1+t);
    size_t l;
    if(1!=sscanf(argv[2],"%lu%c",&l,v))
     {fprintf(stderr,"Limit_Size NOT a Integer: %s\n",argv[2]);
      return(-2);
     }
    else
     {snprintf(v,e+1,"%s/%s",d,b);
      int f,o;
      if(-1==(f=open(argv[1],O_RDONLY)))
       {fprintf(stderr,"Open input file failed: %s\n",argv[1]);
        return(-3);
       }
      else
       {char *u=v+e;
        struct stat z;
        assert(-1!=fstat(f,&z));
        const size_t s=z.st_size;
        if(0==s)
         {snprintf(u,t,a,1);
          if(-1==(o=out(v))) return(-5);
          else close(o);
         }
        else
         {e=((1+l)*l/2);
          if(s>e)
           {fprintf(stderr,"File_Size = %lu > %lu = ((1+%lu)*%lu/2) = Limit_Size_Capacity\n",s,e,l,l);
            return(-4);
           }
          else
           {const void *m;
            assert(MAP_FAILED!=(m=mmap(NULL,s,PROT_READ,MAP_SHARED,f,0)));
            double k=0.5f+(double)l; /* ((1+l)*l/2)>=s -> k*k-2s>=0 */
            const size_t n=ceil(k-(sqrt(k*k-2*s))); /* <- (l+l+1-n)*n/2>=s */
            assert(999999999>=n); /* s>0 -> n>=1 */
            a[i+2]='0'+(1+log10(n));
            k=(double)s/n;
            const size_t r=n%2?ceil(k)-(n-1)/2:ceil(k-0.5f)+1-n/2; /* ((r-1)+..+(r+n-2)) < s <= ((r)+..+(r+n-1)) */
            const uint8_t *p=m;
            for(i=1;i<=n;i++)
             {e=(i!=n?(r+i):(s-((2*r+n)*(n-1)/2)));
              snprintf(u,t,a,i);
              if(-1==(o=out(v))) return(-5);
              else
               {assert((size_t)write(o,p,e)==e);
                close(o);
                p+=e;
               }
             }
            munmap((void*)m,s);
            close(f);
            assert((size_t)(p-(uint8_t*)m)==s); /* n:=t */
           }
         }
        return(0);
       }
     }
   }
 }

