
#include "minihv.h"

/* titel: malloc()/free()-Paar nach K&R 2, p.185ff */

typedef long Align;

union header
{			/* Kopf eines Allokationsblocks */
    struct
    {
        union header	*ptr;  	/* Zeiger auf zirkulaeren Nachfolger */
        unsigned 	size;	/* Groesse des Blocks	*/
    } s;
    Align x;			/* Erzwingt Block-Alignierung	*/
};

typedef union header Header;

static Header base;		/* Anfangs-Header	*/
static Header *freep = NULL;	/* Aktueller Einstiegspunkt in Free-Liste */

void* malloc(unsigned nbytes);

#define NALLOC 	1024	/* Mindestgroesse fuer morecore()-Aufruf	*/

/* Eine static-Funktion ist ausserhalb ihres Files nicht sichtbar	*/

static Header *morecore(unsigned nu);

void free(void *ap);


