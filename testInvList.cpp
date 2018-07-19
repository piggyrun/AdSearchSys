#include <stdio.h>
#include <stdlib.h>
#include "AdSearchDef.h"

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("usage:\n%s filename offset\n", argv[0]);
		return -1;
	}
	printf("size of ad struct is %d.\n", sizeof(AdSearchRecord));

	FILE * pf = fopen(argv[1],"r");
	if (pf == NULL)
	{
		printf("can't open date file.\n");
		return -1;
	}

	AdSearchRecord ad;
	unsigned int offset = atoi(argv[2]);
	fseek(pf, offset*sizeof(AdSearchRecord), SEEK_SET);
	fread(&ad, sizeof(AdSearchRecord), 1, pf);

	printf("ad:\nquery: %s\nadid: %d\naccid: %d\nregion: %s\npos: %d\nsrc: %s\ncate: %d\nadkey: %s\nisTest: %d\n",
			ad.query, ad.adid, ad.accid, ad.region, ad.pos, ad.src, ad.cate, ad.adkey, ad.isTest);

	fclose(pf);
	return 0;
}
