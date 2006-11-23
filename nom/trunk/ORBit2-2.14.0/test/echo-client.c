/*
 * CORBA echo test
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Elliot Lee <sopwith@redhat.com>
 */
#include <stdio.h>
#include <stdlib.h>

#include "echo.h"


#define ABORT_IF_EXCEPTION(_ev, _message)                    \
if ((_ev)->_major != CORBA_NO_EXCEPTION) {                   \
  g_error("%s: %s", _message, CORBA_exception_id (_ev));     \
  CORBA_exception_free (_ev);                                \
  abort();                                                   \
}

static Echo echo_client, bec;

gboolean echo_opt_quiet = FALSE;

int
main (int argc, char *argv[]) {
	CORBA_Environment ev;
	CORBA_ORB orb;
	CORBA_double rv;
	char buf[30];
	int i;

	int niters = 1000;

	CORBA_exception_init(&ev);
	orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

	/* read IOR from command line as first argument */
	if(argc < 2) {
		g_print ("ERROR, usage: %s <ior> [<#iterations>]\n", argv[0]);
		return 1;
	}

	if (argv[1][0] == '$')
	  argv[1] = getenv (argv[1]+1);

	/* read (optional) number of iterations from command line as
	 * second argument (100) */    
	if(argc == 3)
		niters = atoi(argv[2]);
    
	/* bind to object */
	echo_client = CORBA_ORB_string_to_object(orb, argv[1], &ev);
	ABORT_IF_EXCEPTION (&ev, "cannot bind to object");
	g_assert (echo_client!=NULL);

	/* Iterate various times.  Each time the client invokes
	 * 'echoString(..)'  a new reference to service is returned which
	 * is used for next loop. At end of each loop the old
	 * obj. reference is released. */
	for(i = 0; i < niters; i++) {
		/* Method call without any argument, usefull to tell
		 * lifeness */
		Echo_doNothing(echo_client, &ev);
		ABORT_IF_EXCEPTION (&ev, "service raised exception ");

		/* Ask echo-service to print string 'buf' on console. The
		 * service returns random double float value in 'vr' */
		g_snprintf(buf, sizeof(buf), "Hello, world [%d]", i);
		bec = Echo_echoString(echo_client, buf, &rv, &ev);
		ABORT_IF_EXCEPTION (&ev, "service raised exception ");

		/* print random value generated by echo-service */
		if ( !echo_opt_quiet )
			g_message("[client] %g", rv);

		/* Asynchronous/oneway method call, the function returns
		 * immediately.  Usefull for log-message transfer */
		Echo_doOneWay(echo_client, "log message ", &ev);
		ABORT_IF_EXCEPTION (&ev, "service raised exception ");

		/* release first object reference and use the new one for
		 * next loop */
		CORBA_Object_release(echo_client, &ev);
		ABORT_IF_EXCEPTION (&ev, "service raised exception ");

		/* swap object references */ 
		echo_client = bec; bec = CORBA_OBJECT_NIL;
	}
    
	/* release initial object reference */
	CORBA_Object_release(echo_client, &ev);
	ABORT_IF_EXCEPTION (&ev, "service raised exception ");

	/* shutdown ORB, shutdown IO channels */
	CORBA_ORB_shutdown (orb, FALSE, &ev);
	ABORT_IF_EXCEPTION(&ev, "ORB shutdown ...");

	/* destroy local ORB */
	CORBA_ORB_destroy(orb, &ev);
	ABORT_IF_EXCEPTION (&ev, "destroying local ORB raised exception");

	return 0;
}
