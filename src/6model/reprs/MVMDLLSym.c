#include "moar.h"

static const MVMREPROps this_repr;

static MVMObject * type_object_for(MVMThreadContext *tc, MVMObject *HOW) {
    MVMSTable *st = MVM_gc_allocate_stable(tc, &this_repr, HOW);

    MVMROOT(tc, st, {
        MVMObject *obj = MVM_gc_allocate_type_object(tc, st);
        MVM_ASSIGN_REF(tc, &(st->header), st->WHAT, obj);
        st->size = sizeof(MVMDLLSym);
    });

    return st->WHAT;
}

static void initialize(MVMThreadContext *tc, MVMSTable *st, MVMObject *root,
        void *data) {
    MVMDLLSymBody *body = data;

    body->address = NULL;
    body->dll = NULL;
}

static void copy_to(MVMThreadContext *tc, MVMSTable *st, void *src,
        MVMObject *dest_root, void *dest) {
    MVMDLLSymBody *src_body = src;
    MVMDLLSymBody *dest_body = dest;

    *dest_body = *src_body;

    if (dest_body->dll)
        MVM_incr(&dest_body->dll->refcount);
}

static MVMStorageSpec get_storage_spec(MVMThreadContext *tc, MVMSTable *st) {
    MVMStorageSpec spec;

    spec.inlineable      = MVM_STORAGE_SPEC_REFERENCE;
    spec.boxed_primitive = MVM_STORAGE_SPEC_BP_NONE;
    spec.can_box         = 0;

    return spec;
}

static void compose(MVMThreadContext *tc, MVMSTable *st, MVMObject *info) {
    /* noop */
}

const MVMREPROps * MVMDLLSym_initialize(MVMThreadContext *tc) {
    MVMSTable *st = MVM_gc_allocate_stable(tc, &this_repr, NULL);

    MVMROOT(tc, st, {
        MVMObject *WHAT = MVM_gc_allocate_type_object(tc, st);
        tc->instance->raw_types.RawDLLSym = WHAT;
        MVM_ASSIGN_REF(tc, &(st->header), st->WHAT, WHAT);
        st->size = sizeof(MVMDLLSym);
    });

    MVM_gc_root_add_permanent(tc,
            (MVMCollectable **)&tc->instance->raw_types.RawDLLSym);

    return &this_repr;
}

static const MVMREPROps this_repr = {
    type_object_for,
    MVM_gc_allocate_object,
    initialize,
    copy_to,
    MVM_REPR_DEFAULT_ATTR_FUNCS,
    MVM_REPR_DEFAULT_BOX_FUNCS,
    MVM_REPR_DEFAULT_POS_FUNCS,
    MVM_REPR_DEFAULT_ASS_FUNCS,
    MVM_REPR_DEFAULT_ELEMS,
    get_storage_spec,
    NULL, /* change_type */
    NULL, /* serialize */
    NULL, /* deserialize */
    NULL, /* serialize_repr_data */
    NULL, /* deserialize_repr_data */
    NULL, /* deserialize_stable_size */
    NULL, /* gc_mark */
    NULL, /* gc_free */
    NULL, /* gc_cleanup */
    NULL, /* gc_mark_repr_data */
    NULL, /* gc_free_repr_data */
    compose,
    NULL, /* spesh */
    "MVMDLLSym",
    MVM_REPR_ID_MVMDLLSym,
    0, /* refs_frames */
};
