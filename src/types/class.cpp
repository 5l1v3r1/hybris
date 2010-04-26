/*
 * This file is part of the Hybris programming language interpreter.
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
#include "common.h"
#include "types.h"
#include "node.h"
#include "mseg.h"
#include "vm.h"

/** helpers **/
Object *class_call_overloaded_operator( Object *me, const char *op_name, int argc, ... ){
	char     op_mangled_name[0xFF] = {0};
	Node    *op = H_UNDEFINED;
	vframe_t stack,
			*frame;
	Object  *result               = H_UNDEFINED;
	unsigned int i, op_argc;
	va_list ap;
	extern VM __hyb_vm;

	sprintf( op_mangled_name, "__op@%s", op_name );

	if( (op = ob_get_method( me, op_mangled_name, argc )) == H_UNDEFINED ){
		hyb_error( H_ET_SYNTAX, "class %s does not overload '%s' operator", ob_typename(me), op_name );
	}

	op_argc = op->children() - 1;

	/*
	 * The last child of a method is its body itself, so we compare
	 * call children with method->children() - 1 to ignore the body.
	 */
	if( argc != op_argc ){
		__hyb_vm.depool();
		hyb_error( H_ET_SYNTAX, "operator '%s' requires %d parameters (called with %d)",
								 op_name,
								 op_argc,
								 argc );
	}

	/*
	 * Create the "me" reference to the class itself, used inside
	 * methods for me->... calls.
	 */
	stack.insert( "me", me );
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		/*
		 * Value references count is set to zero now, builtins
		 * do not care about reference counting, so this object will
		 * be safely freed after the method call by the gc.
		 */
		stack.insert( (char *)op->child(i)->value.m_identifier.c_str(), va_arg( ap, Object * ) );
	}
	va_end(ap);

	/*
	 * Save current frame.
	 */
	frame = __hyb_vm.vframe;

	__hyb_vm.trace( (char *)op_name, &stack );
	__hyb_vm.setCurrentFrame( &stack );

	/* call the operator */
	result = __hyb_vm.engine->exec( &stack, op->callBody() );

	__hyb_vm.setCurrentFrame( frame );
	__hyb_vm.detrace();

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state._exception == true ){
		stack.state._exception = false;
		__hyb_vm.vframe->state._exception = true;
		__hyb_vm.vframe->state.value 	  = stack.state.value;
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

Object *class_call_overloaded_descriptor( Object *me, const char *ds_name, bool lazy, int argc, ... ){
	Node    *ds = H_UNDEFINED;
	vframe_t stack,
			*frame;
	Object  *result = H_UNDEFINED;
	unsigned int i, ds_argc;
	va_list ap;
	extern VM __hyb_vm;
	vframe_state_t state;

	if( (ds = ob_get_method( me, (char *)ds_name, argc )) == H_UNDEFINED ){
		if( lazy == false ){
			hyb_error( H_ET_SYNTAX, "class %s does not overload '%s' descriptor", ob_typename(me), ds_name );
		}
		else{
			return H_UNDEFINED;
		}
	}

	ds_argc = ds->children() - 1;

	/*
	 * The last child of a method is its body itself, so we compare
	 * call children with method->children() - 1 to ignore the body.
	 */
	if( argc != ds_argc ){
		__hyb_vm.depool();
		hyb_error( H_ET_SYNTAX, "descriptor '%s' requires %d parameters (called with %d)",
								 ds_name,
								 ds_argc,
								 argc );
	}

	/*
	 * Create the "me" reference to the class itself, used inside
	 * methods for me->... calls.
	 */
	stack.insert( "me", me );
	va_start( ap, argc );
	for( i = 0; i < argc; ++i ){
		/*
		 * Value references count is set to zero now, builtins
		 * do not care about reference counting, so this object will
		 * be safely freed after the method call by the gc.
		 */
		stack.insert( (char *)ds->child(i)->value.m_identifier.c_str(), va_arg( ap, Object * ) );
	}
	va_end(ap);

	/*
	 * Save current frame.
	 */
	frame = __hyb_vm.vframe;
	/*
	 * Save stack frame state.
	 */
	state.assign(__hyb_vm.vframe->state);
	/*
	 * Reset it.
	 */
	__hyb_vm.vframe->state.reset();

	/* call the descriptor */
	result = __hyb_vm.engine->exec( &stack, ds->callBody() );
	/*
	 * Restore stack frame state.
	 */
	__hyb_vm.vframe->state.assign(state);

	/*
	 * Check for unhandled exceptions and put them on the root
	 * memory frame.
	 */
	if( stack.state._exception == true ){
		stack.state._exception = false;
		__hyb_vm.vframe->state._exception = true;
		__hyb_vm.vframe->state.value 	  = stack.state.value;
	}

	/* return method evaluation value */
	return (result == H_UNDEFINED ? H_DEFAULT_RETURN : result);
}

/** generic function pointers **/
const char *class_typename( Object *o ){
	return ob_class_ucast(o)->name.c_str();
}

void class_set_references( Object *me, int ref ){
    ClassObjectAttributeIterator ai;
    ClassObject *cme = ob_class_ucast(me);

    me->ref += ref;

    for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
        ob_set_references( (*ai)->value->value, ref );
    }
}

Object *class_clone( Object *me ){
    ClassObject *cclone = gc_new_class(),
                *cme    = ob_class_ucast(me);
    ClassObjectAttributeIterator ai;
    ClassObjectMethodIterator    mi;
    ClassObjectMethodVariationsIterator mvi;

    vector<Node *>				 method_variations;

    for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
    	cclone->c_attributes.insert( (char *)(*ai)->value->name.c_str(),
									 new class_attribute_t(
									   (*ai)->value->name,
									   (*ai)->value->access,
									   ob_clone((*ai)->value->value)
							       )
								 );
    }

    for( mi = cme->c_methods.begin(); mi != cme->c_methods.end(); mi++ ){
    	method_variations.clear();
    	for( mvi = (*mi)->value->method.begin(); mvi != (*mi)->value->method.end(); mvi++ ){
    		method_variations.push_back( (*mvi)->clone() );
    	}

    	cclone->c_methods.insert( (char *)(*mi)->value->name.c_str(),
								  new class_method_t(
									(*mi)->value->name,
									method_variations
								  )
							    );
    }

    cclone->name  = cme->name;

    return (Object *)(cclone);
}

size_t class_get_size( Object *me ){
	Object *size = class_call_overloaded_descriptor( me, "__size", false, 0 );
	return ob_ivalue(size);
}

void class_free( Object *me ){
    ClassObjectAttributeIterator ai;
    ClassObjectMethodIterator    mi;
    ClassObjectMethodVariationsIterator mvi;
    ClassObject *cme = ob_class_ucast(me);
    class_method_t *method;
    class_attribute_t *attribute;

    /*
     * Check if the class has a destructors and call it.
     */
    if( (method = cme->c_methods.find( "__expire" )) ){
		for( mvi = method->method.begin(); mvi != method->method.end(); mvi++ ){
			Node *dtor = (*mvi);
			vframe_t stack;
			extern VM __hyb_vm;

			stack.insert( "me", me );

			__hyb_vm.trace( "__expire", &stack );

			__hyb_vm.engine->exec( &stack, dtor->callBody() );

			__hyb_vm.detrace();
		}
    }
	/*
	 * Delete c_methods structure pointers.
	 */
	for( mi = cme->c_methods.begin(); mi != cme->c_methods.end(); mi++ ){
		method = (*mi)->value;
		delete method;
	}
	cme->c_methods.clear();
	/*
	 * Delete c_attributes structure pointers and decrement values references.
	 */
	for( ai = cme->c_attributes.begin(); ai != cme->c_attributes.end(); ai++ ){
		attribute = (*ai)->value;
		if( attribute->value ){
			ob_free( attribute->value );
		}
		delete attribute;
	}
	cme->c_attributes.clear();
}

long class_ivalue( Object *me ){
    return static_cast<long>( class_get_size(me) );
}

double class_fvalue( Object *me ){
    return static_cast<double>( class_get_size(me) );
}

bool class_lvalue( Object *me ){
    return static_cast<bool>( class_get_size(me) );
}

string class_svalue( Object *me ){
	Object *svalue = H_UNDEFINED;

	if( (svalue = class_call_overloaded_descriptor( me, "__to_string", true, 0 )) == H_UNDEFINED ){
		return "<" + ob_class_ucast(me)->name + ">";
	}
	else{
		return ob_svalue(svalue);
	}
}

void class_print( Object *me, int tabs ){
	int j;
	Object *svalue = class_call_overloaded_descriptor( me, "__to_string", false, 0 );

	ob_print( svalue, tabs );
}

Object *class_range( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "..", 1, op );
}

Object *class_regexp( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "~=", 1, op );
}

/** arithmetic operators **/
Object *class_assign( Object *me, Object *op ){
    class_free(me);

    Object *clone = ob_clone(op);

    ob_set_references( clone, +1 );

    return (me = clone);
}

Object *class_increment( Object *me ){
	return class_call_overloaded_operator( me, "++", 0 );
}

Object *class_decrement( Object *me ){
	return class_call_overloaded_operator( me, "--", 0 );
}

Object *class_add( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "+", 1, op );
}

Object *class_sub( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "-", 1, op );
}

Object *class_mul( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "*", 1, op );
}

Object *class_div( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "/", 1, op );
}

Object *class_mod( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "%", 1, op );
}

Object *class_inplace_add( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "+=", 1, op );
}

Object *class_inplace_sub( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "-=", 1, op );
}

Object *class_inplace_mul( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "*=", 1, op );
}

Object *class_inplace_div( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "/=", 1, op );
}

Object *class_inplace_mod( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "%=", 1, op );
}

/** bitwise operators **/
Object *class_bw_and( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "&", 1, op );
}

Object *class_bw_or( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "|", 1, op );
}

Object *class_bw_not( Object *me ){
	return class_call_overloaded_operator( me, "~", 0 );
}

Object *class_bw_xor( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "^", 1, op );
}

Object *class_bw_lshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<<", 1, op );
}

Object *class_bw_rshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">>", 1, op );
}

Object *class_bw_inplace_and( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "&=", 1, op );
}

Object *class_bw_inplace_or( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "|=", 1, op );
}

Object *class_bw_inplace_xor( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "^=", 1, op );
}

Object *class_bw_inplace_lshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<<=", 1, op );
}

Object *class_bw_inplace_rshift( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">>=", 1, op );
}

/** logic operators **/
Object *class_l_not( Object *me ){
	return class_call_overloaded_operator( me, "!", 0 );
}

Object *class_l_same( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "==", 1, op );
}

Object *class_l_diff( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "!=", 1, op );
}

Object *class_l_less( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<", 1, op );
}

Object *class_l_greater( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">", 1, op );
}

Object *class_l_less_or_same( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "<=", 1, op );
}

Object *class_l_greater_or_same( Object *me, Object *op ){
	return class_call_overloaded_operator( me, ">=", 1, op );
}

Object *class_l_or( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "||", 1, op );
}

Object *class_l_and( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "&&", 1, op );
}

/* collection operators */
Object *class_cl_push( Object *me, Object *op ){
	return class_call_overloaded_operator( me, "[]=", 1, op );
}

Object *class_cl_at( Object *me, Object *index ){
	return class_call_overloaded_operator( me, "[]", 1, index );
}

Object *class_cl_set( Object *me, Object *index, Object *op ){
	return class_call_overloaded_operator( me, "[]<", 2, index, op );
}

/** class operators **/
void class_define_attribute( Object *me, char *name, access_t a ){
	ClassObject *cme = ob_class_ucast(me);

	cme->c_attributes.insert( name,
							  new class_attribute_t(
									  name,
									  a,
									  (Object *)gc_new_integer(0)
							  )
							);
}

access_t class_attribute_access( Object *me, char *name ){
	ClassObject *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		return attribute->access;
	}

	return asPublic;
}

void class_set_attribute_access( Object *me, char *name, access_t a ){
	ClassObject *cme = ob_class_ucast(me);
	class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		attribute->access = a;
	}
}

void class_add_attribute( Object *me, char *name ){
	class_define_attribute( me, name, asPublic );
}

Object *class_get_attribute( Object *me, char *name, bool with_descriptor /* = true */ ){
    ClassObject *cme = ob_class_ucast(me);
    class_attribute_t *attribute;
    Object *a_value;

    /*
     * If the attribute is defined, return it.
     */
	if( (attribute = cme->c_attributes.find(name)) != H_UNDEFINED ){
		return attribute->value;
	}
	/*
	 * Else, if the class overloads the __attribute descriptor,
	 * call it.
	 */
	else if( with_descriptor && (a_value = class_call_overloaded_descriptor( me, "__attribute", true, 1, (Object *)gc_new_string(name) )) != H_UNDEFINED ){
		return a_value;
	}
	/*
	 * Nothing found.
	 */
	else{
		return NULL;
	}
}

void class_set_attribute_reference( Object *me, char *name, Object *value ){
    ClassObject *cme = ob_class_ucast(me);
    class_attribute_t *attribute;

	if( (attribute = cme->c_attributes.find(name)) != NULL ){
		/*
		 * Set the new value, ob_assign will decrement old value
		 * reference counter.
		 */
		attribute->value = ob_assign( attribute->value, value );
	}
	else{
		class_call_overloaded_descriptor( me, "__attribute", false, 2, (Object *)gc_new_string(name), value );
	}
}

void class_set_attribute( Object *me, char *name, Object *value ){
    return ob_set_attribute_reference( me, name, ob_clone(value) );
}

void class_define_method( Object *me, char *name, Node *code ){
	ClassObject *cme = ob_class_ucast(me);
	class_method_t *method;
	/*
	 * Check if there's already a method with that name, in this case
	 * push the node to the variations vector.
	 */
	if( (method = cme->c_methods.find(name)) ){
		method->method.push_back( code->clone() );
	}
	/*
	 * Otherwise define a new method.
	 */
	else{
		cme->c_methods.insert( name, new class_method_t( name, code->clone() ) );
	}
}

Node *class_get_method( Object *me, char *name, int argc ){
	ClassObject *cme = ob_class_ucast(me);
	class_method_t *method;
	char mangled_op_name[0xFF] = {0};

	sprintf( mangled_op_name, "__op@%s", name );

	if( (method = cme->c_methods.find(name)) || (method = cme->c_methods.find(mangled_op_name)) ){
		/*
		 * If no parameters number is specified, return the first method found.
		 */
		if( argc < 0 ){
			return (*method->method.begin());
		}
		/*
		 * Otherwise, find the best match.
		 */
		ClassObjectMethodVariationsIterator mvi;
		Node *best_match = NULL;
		int   best_match_argc, match_argc;

		for( mvi = method->method.begin(); mvi != method->method.end(); mvi++ ){
			/*
			 * The last child of a method is its body itself, so we compare
			 * call children with method->children() - 1 to ignore the body.
			 */
			if( best_match == NULL ){
				best_match 		= *mvi;
				best_match_argc = best_match->children() - 1;
			}
			else{
				match_argc = (*mvi)->children() - 1;
				if( match_argc != best_match_argc && match_argc == argc ){
					return (*mvi);
				}
			}
		}

		return best_match;
	}
	else{
		return NULL;
	}
}

IMPLEMENT_TYPE(Class) {
    /** type code **/
    otClass,
	/** type name **/
    "class",
	/** type basic size **/
    0,

	/** generic function pointers **/
    class_typename, // type_name
	class_set_references, // set_references
	class_clone, // clone
	class_free, // free
	class_get_size, // get_size
	0, // serialize
	0, // deserialize
	0, // to_fd
	0, // from_fd
	0, // cmp
	class_ivalue, // ivalue
	class_fvalue, // fvalue
	class_lvalue, // lvalue
	class_svalue, // svalue
	class_print, // print
	0, // scanf
	0, // to_string
	0, // to_int
	0, // from_int
	0, // from_float
	class_range, // range
	class_regexp, // regexp

	/** arithmetic operators **/
	class_assign, // assign
    0, // factorial
    class_increment, // increment
    class_decrement, // decrement
    0, // minus
    class_add, // add
    class_sub, // sub
    class_mul, // mul
    class_div, // div
    class_mod, // mod
    class_inplace_add, // inplace_add
    class_inplace_sub, // inplace_sub
    class_inplace_mul, // inplace_mul
    class_inplace_div, // inplace_div
    class_inplace_mod, // inplace_mod

	/** bitwise operators **/
	class_bw_and, // bw_and
    class_bw_or, // bw_or
    class_bw_not, // bw_not
    class_bw_xor, // bw_xor
    class_bw_lshift, // bw_lshift
    class_bw_rshift, // bw_rshift
    class_bw_inplace_and, // bw_inplace_and
    class_bw_inplace_or, // bw_inplace_or
    class_bw_inplace_xor, // bw_inplace_xor
    class_bw_inplace_lshift, // bw_inplace_lshift
    class_bw_inplace_rshift, // bw_inplace_rshift

	/** logic operators **/
    class_l_not, // l_not
    class_l_same, // l_same
    class_l_diff, // l_diff
    class_l_less, // l_less
    class_l_greater, // l_greater
    class_l_less_or_same, // l_less_or_same
    class_l_greater_or_same, // l_greater_or_same
    class_l_or, // l_or
    class_l_and, // l_and

	/** collection operators **/
	0, // cl_concat
	0, // cl_inplace_concat
	class_cl_push, // cl_push
	0, // cl_push_reference
	0, // cl_pop
	0, // cl_remove
	class_cl_at, // cl_at
	class_cl_set, // cl_set
	0, // cl_set_reference

	/** structure operators **/
	class_define_attribute, // define_attribute
	class_attribute_access, // attribute_access
	class_set_attribute_access, // set_attribute_access
    class_add_attribute, // add_attribute
    class_get_attribute, // get_attribute
    class_set_attribute, // set_attribute
    class_set_attribute_reference,  // set_attribute_reference
    class_define_method, // define_method
    class_get_method  // get_method
};

