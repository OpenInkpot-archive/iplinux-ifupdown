#line 450 "ifupdown.nw"
#include <stdlib.h>
#include "header.h"

#line 3825 "ifupdown.nw"
extern address_family addr_inet;
#line 4005 "ifupdown.nw"
extern address_family addr_inet6;
#line 4076 "ifupdown.nw"
extern address_family addr_ipx;

#line 455 "ifupdown.nw"
address_family *addr_fams[] = {
	
#line 3829 "ifupdown.nw"
&addr_inet, 
#line 4009 "ifupdown.nw"
&addr_inet6,
#line 4080 "ifupdown.nw"
&addr_ipx,
#line 457 "ifupdown.nw"
	NULL
};
