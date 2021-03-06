/*
 * This file is part of the Hybris programming language.
 *
 * Copyleft of Simone Margaritelli aka evilsocket <evilsocket@gmail.com>
 *
 * Hybris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hybris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hybris.  If not, see <http://www.gnu.org/licenses/>.
*/

import std.io.network.tcp;
import std.os.process;
import std.lang.map;

class CGI {
	protected env;

	static stdout = STDOUT_FILENO;
	static stdin  = STDIN_FILENO;

	method CGI(){
		me.env = env();
	
		if( me.env.has( "REQUEST_METHOD" ) ){
			if( me.env["REQUEST_METHOD"] == "POST" ){
				me.env["POST"] = map();	

				data = " ";
				recv( CGI.stdin, data );
				
				duples = data.split("&");
				foreach( duple of duples ){
					( name, value ) = duple.split("=");
					me.env["POST"][name] = value;
				}	
			}
		}

		if( me.env.has( "QUERY_STRING" ) != -1 ){
			me.env["GET"] = map();	
			data = me.env["QUERY_STRING"];
						
			if( data.trim() != "" ){
				duples = data.trim().split("&");
				foreach( duple of duples ){
					( name, value ) = duple.split("=");
					me.env["GET"][name] = value;
				}		
			}
		}
	}

	method CGI( type ){
		me.CGI();
		send( CGI.stdout,  "Content-type: " + type + "\n\n" );
	}

	method env(){
		return me.env;
	}

	method __attribute( name ){
		value = null;		
		if( haskey( me.env, name ) != -1 ){
			value = me.env[name];
		}	
		return value;
	}

	operator << ( data ){
		send( __CGI_stdout, tostring(data) );
		return me;
	}
}
