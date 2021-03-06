/* Callsite argument flags. */
#define MVM_CALLSITE_ARG_MASK 31
typedef enum {
    /* Argument is an object. */
    MVM_CALLSITE_ARG_OBJ = 1,

    /* Argument is a native integer, signed. */
    MVM_CALLSITE_ARG_INT = 2,

    /* Argument is a native floating point number. */
    MVM_CALLSITE_ARG_NUM = 4,

    /* Argument is a native NFG string (MVMString REPR). */
    MVM_CALLSITE_ARG_STR = 8,

    /* Argument is named; in this case, there are two entries in
     * the argument list, the first a MVMString naming the arg and
     * after that the arg. */
    MVM_CALLSITE_ARG_NAMED = 32,

    /* Argument is flattened. What this means is up to the target. */
    MVM_CALLSITE_ARG_FLAT = 64,

    /* Argument is flattened and named. */
    MVM_CALLSITE_ARG_FLAT_NAMED = 128
} MVMCallsiteFlags;

/* A callsite entry is just one of the above flags. */
typedef MVMuint8 MVMCallsiteEntry;

/* A callsite is an argument count and a bunch of flags. Note that it
 * does not contain the values; this is the *statically known* things
 * about the callsite and is immutable. It describes how to process
 * the callsite memory buffer. */
struct MVMCallsite {
    /* The set of flags. */
    MVMCallsiteEntry *arg_flags;

    /* The total argument count (including 2 for each
     * named arg). */
    MVMuint16 arg_count;

    /* Number of positionals. */
    MVMuint16 num_pos;

    /* Whether it has any flattening args. */
    MVMuint8 has_flattening;

    /* Whether it has been interned (which means it is suitable for using in
     * specialization). */
    MVMuint8 is_interned;

    /* Cached version of this callsite with an extra invocant arg. */
    MVMCallsite *with_invocant;

#if MVM_HLL_PROFILE_CALLS
    MVMuint32 static_frame_id;
    MVMuint8 *cuuid, *name;
#endif
};

/* Minimum callsite size is due to certain things internally expecting us to
 * have that many slots available (e.g. find_method(how, obj, name)). */
#define MVM_MIN_CALLSITE_SIZE 3

/* Maximum arity + 1 that we'll intern callsites by. */
#define MVM_INTERN_ARITY_LIMIT 8

/* Interned callsites data structure. */
struct MVMCallsiteInterns {
    /* Array of callsites, by arity. */
    MVMCallsite **by_arity[MVM_INTERN_ARITY_LIMIT];

    /* Number of callsites we have interned by arity. */
    MVMint32 num_by_arity[MVM_INTERN_ARITY_LIMIT];
};

/* Callsite interning function. */
void MVM_callsite_try_intern(MVMThreadContext *tc, MVMCallsite **cs);
