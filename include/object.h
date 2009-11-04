#ifndef _HOBJECT_H_
#	define _HOBJECT_H_

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>

extern void  hybris_syntax_error( const char *format, ... );
extern void  hybris_generic_error( const char *format, ... );

using std::string;
using std::stringstream;
using std::vector;
using std::cout;
using std::cin;

typedef unsigned short H_OBJECT_TYPE;

#define H_OT_NONE   0
#define H_OT_INT    1
#define H_OT_FLOAT  2
#define H_OT_CHAR   3
#define H_OT_STRING 4
#define H_OT_ARRAY  5
#define H_OT_MAP    6
#define H_OT_ALIAS  7

class Object {
public  :
    /* return a string rapresentation of the type */
    static const char *type( Object *o );
    /* create an object from a xml tree */
	static Object *fromxml( xmlNode *node );
    /* create an object from a xml stream */
	static Object *fromxml( char *xml );
    /* return 1 if a and b are of one of the types described on the arguments */
	static unsigned int assert_type( Object *a, Object *b, unsigned int ntypes, ... );
    /* replace each occurrence of "find" in "source" with "replace" */
    static void replace( string &source, const string find, string replace );

    static void parse_string( string& s );

    H_OBJECT_TYPE    xtype;
    unsigned int     xsize;
    unsigned int     is_extern;

    int              xint;
    double           xfloat;
    char             xchar;
    string           xstring;
	vector<Object *> xarray;
	vector<Object *> xmap;
	unsigned int     xalias;
	
    Object( int value );
    Object( int value, unsigned int _is_extern );
    Object( double value );
    Object( char value );
    Object( char *value );
	Object();
	Object( unsigned int value );
    Object( Object *o );
    Object( FILE *fp );

    ~Object();

	int equals( Object *o );

	int mapFind( Object *map );
	
	Object * getObject();

    void compile( FILE *fp );
    unsigned char *serialize();

    void print( unsigned int tabs = 0 );

    void println( unsigned int tabs = 0 );

	string toxml( unsigned int tabs = 0 );
	
    void input();
	Object * toString();
    
	int lvalue();
    Object * dot( Object *o );
	Object * dotequal( Object *o );  
    string svalue();
	Object *push( Object *o );
	Object *map( Object *map, Object *o );
	Object *pop();
	Object *mapPop();
	Object *remove( Object *index );
	Object *unmap( Object *map );
    Object *at( Object *index );
	Object *at( char *map );
    Object& at( Object *index, Object *set );
	
    Object&  operator = ( Object *o );
    Object * operator - ();
    Object * operator ! ();
    Object&  operator ++ ();
    Object&  operator -- ();
    Object * operator + ( Object *o );
	Object * operator += ( Object *o );
    Object * operator - ( Object *o );
	Object * operator -= ( Object *o );
    Object * operator * ( Object *o );
	Object * operator *= ( Object *o );
    Object * operator / ( Object *o );
	Object * operator /= ( Object *o );
    Object * operator % ( Object *o );
	Object * operator %= ( Object *o );
    Object * operator & ( Object *o );
	Object * operator &= ( Object *o );
    Object * operator | ( Object *o );
	Object * operator |= ( Object *o );
	Object * operator << ( Object *o );
	Object * operator <<= ( Object *o );
	Object * operator >> ( Object *o );
	Object * operator >>= ( Object *o );
    Object * operator ^ ( Object *o );
	Object * operator ^= ( Object *o );
    Object * operator ~ ();
	Object * lnot ();
    Object * operator == ( Object *o );
    Object * operator != ( Object *o );
    Object * operator < ( Object *o );
    Object * operator > ( Object *o );
    Object * operator <= ( Object *o );
    Object * operator >= ( Object *o );
    Object * operator || ( Object *o );
    Object * operator && ( Object *o );
};

#endif
