# wholang

## Thoughts
I've decided that the early goal for this language is something that could be used to replace C. Not in the manner that
it is a well established and widely documented language, but the fact that it doesn't have much frills or fancy things.
No garbage collection, no (actual) OOP, none of that fancy shit that C++ was ruined with. My rule of thumb is if I can't
make a x86 kernel with the language and some assembly, it's fucking useless. However, I would like to try to implement
some nice syntax sugar that has come up from other languages. If some of these features seem like they come from other
languages, that's because they do, and I thought they looked pretty and wanted them. I guess that makes this a language
a frankenstein of others. Except frankenstein is a shit word. Let's call it a pretty quilt of other languages :).

## Design
Please note that all of this is completely open to change and that if someone thinks of a better idea, then we'll
probably move towards it. So these are the accumulated thoughts of far:

    #include <global.wh>  // Look familiar?
    #include "local.wh"   /* it should. \*/

    fn main(argc: int, argv: const str[]) : int { // str == char* :) and explicit return types.
        var test = fn () : int { // Implicit variable types
            return 0;
        }; // Anonymous functions

        var funnies : int = test(); // And explicit variable types

        // Automatic look ahead ;)
        var arr : stringArray* = init stringArray; // Syntaxual sugar for (stringArray*) malloc(sizeof(StringArray)); StringArray___alloc__();

        // Syntaxual sugar for StringArray_add(arr, "test");
        arr:add("test");


        for ( var i = 0; i < 10; i++ ) {
            // blah blah blah
        }

        while ( butts == funny ) {
            // i think you get it.
        }

        return funnies;
    }

    struct /* A name could be here like stringArray or some shit, but it would need to be referenced as a struct \*/ {
        length : int,
        alloc : int,
        data : str*
    } : stringArray; // : to typedef.

    // Syntaxual sugar for fn stringArray_add(StringArray* self, str input)
    fn stringArray:add(input) { // implicitly void return type
        // blah blah
    }

    fn stringArray:\__alloc\__() { // Called when a struct is allocated via init.
        this->alloc = 8;
        this->data = [str*] malloc(sizeof(str) * this->alloc); // NOTE: str is a pointer! If we were allocating for a string it'd need to be char!
        this->length = 0;
    }

This is currently what I have planned out. Might not be much, but it's enough to get started with a turing complete
language going.
