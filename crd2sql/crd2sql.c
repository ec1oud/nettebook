/* 
 *
 * Crd2sql -- convert Windows card files to sql insert commands 
 * 				for NetteBook
 * Based on: Crd2txt by Hannu.Aronsson@iki.fi
 *
 * File format looks basically like this (see code for details). This
 * Interpretation has been tested on card files of up to about 1100
 * cards and 97k of data total.
 * 
 * Header: Contains magic number (3 bytes) and number of cards.
 *
 * Title records: number of card times from offset 11 are 52-byte
 * records with rest-of-text offset in 2 shorts (within-page
 * address and page number) and the title text (terminated by NULL).
 *
 * Rest of text records: At indicated offset, there is a text length
 * short and then the text, with CRLF as line separator.
 *
 * Character set in all strings is Windows, i.e. about Latin1 8859-1
 *
 * Single-quote characters in the card fields must be escaped
 * in the INSERT command
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

void seeknread(int f, unsigned long where, 
               unsigned char *buf, unsigned long len)
{
  int r;

  r = lseek(f, where, SEEK_SET);
  assert(r == where);

  r = read(f, buf, len);
  assert(r == len);
}

unsigned short readshort(int f, unsigned long where)
{
  unsigned char buf[2];

  seeknread(f, where, buf, 2);
  return buf[0] + buf[1]*256;   /* Windoze is big endian */
}

#define MAXTEXT 2048

main(int ac, char **av)
{
  unsigned char buf[MAXTEXT];
  int f;
  int ncards, i, j, textlen, rec, last;
  unsigned long textloc, filelen, loc;
  char* title = malloc(81);

  if (ac < 4)
  {
	fprintf(stderr, "Usage: crd2sql file.crd person_id category\n");
	exit(1);
  }

  f = open(av[1], O_RDONLY);
  assert(f >= 0);

  filelen = lseek(f, 0, SEEK_END);
  assert(filelen > 8);

  seeknread(f, 0, buf, 3);
  assert(!strncmp(buf, "MGC"));
  
  ncards = readshort(f, 3);
  assert(ncards >= 0 && ncards <= 4096);

  fprintf(stderr, "Converting .CRD file %s with %d cards\n",
         av[1], ncards);

  for (rec = 0; rec < ncards; rec++) {
    loc = 11 + rec*52;
    textloc = readshort(f, loc); /* This is address within 64K */
    textloc += readshort(f, loc+2) * 65536; /* This is "page" number */
    assert(textloc > 60 && textloc < filelen);

    seeknread(f, loc+5, buf, 52-5);
    /*    printf("<!-- Converted card %d of %d -->\n", rec+1, ncards); */

	/* Escape any single quotes in the "title" field */
	j = 0;
	for (i = 0; i < 80 && buf[i] != 0; ++i)
	{
		if (buf[i] == 0x27)
		{
			title[j++] = 92;
			title[j] = 0x27;
		}
		else
			title[j] = buf[i];
		++j;
	}
	title[j] = 0;

    printf("INSERT INTO general_purpose VALUES (%s, '%s', '%s', '", 
		av[2], av[3], title);

    textlen = readshort(f, textloc + 2);
    assert(textlen >= 0 && textlen < filelen - textloc &&
           textlen < MAXTEXT);
    
    seeknread(f, textloc + 4, buf, textlen);
    buf[textlen] = '\0';

    last = '\n';
    /* Text har CRLF line endings */
    for (i=0; i< textlen; i++)
      switch(buf[i]) {
      case '\n':
        if (last != '\n')       /* Print only one of consecutive LFs */
          printf("<BR>\n");
        last = '\n';
        break;
      case '\r':        
        /* Ignore, no change to last */
        break;
      case 0x27:        
		printf("\\");
		/* fallthrough */
      default:
        putchar(buf[i]);
        last = buf[i];
      }
    printf("');\n", buf);
  }

  close(f);
}

/* EOF */
