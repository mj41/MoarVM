#include "moar.h"

/* Tries to generate a specialization of the bytecode, for the given callsite
 * and argument tuple. */
MVMSpeshCandidate * MVM_spesh_candidate_generate(MVMThreadContext *tc,
        MVMStaticFrame *static_frame, MVMCallsite *callsite, MVMRegister *args) {
    MVMSpeshCandidate *result;
    MVMSpeshGuard *guards;
    MVMSpeshCode *sc;
    MVMint32 num_spesh_slots, num_guards, *deopts, num_deopts;
    MVMCollectable **spesh_slots;
    char *before, *after;

    /* Generate the specialization. */
    MVMSpeshGraph *sg = MVM_spesh_graph_create(tc, static_frame);
    if (tc->instance->spesh_log_fh)
        before = MVM_spesh_dump(tc, sg);
    MVM_spesh_args(tc, sg, callsite, args);
    MVM_spesh_facts_discover(tc, sg);
    MVM_spesh_optimize(tc, sg);
    if (tc->instance->spesh_log_fh)
        after = MVM_spesh_dump(tc, sg);
    sc = MVM_spesh_codegen(tc, sg);
    num_spesh_slots = sg->num_spesh_slots;
    spesh_slots = sg->spesh_slots;
    num_guards = sg->num_guards;
    guards = sg->guards;
    num_deopts = sg->num_deopt_addrs;
    deopts = sg->deopt_addrs;
    MVM_spesh_graph_destroy(tc, sg);

    /* Now try to add it. Note there's a slim chance another thread beat us
     * to doing so. Also other threads can read the specializations without
     * lock, so make absolutely sure we increment the count of them after we
     * add the new one. */
    result    = NULL;
    uv_mutex_lock(&tc->instance->mutex_spesh_install);
    if (static_frame->body.num_spesh_candidates < MVM_SPESH_LIMIT) {
        MVMint32 num_spesh = static_frame->body.num_spesh_candidates;
        MVMint32 i;
        for (i = 0; i < num_spesh; i++) {
            MVMSpeshCandidate *compare = &static_frame->body.spesh_candidates[i];
            if (compare->cs == callsite && compare->num_guards == num_guards &&
                memcmp(compare->guards, guards, num_guards * sizeof(MVMSpeshGuard)) == 0) {
                /* Beaten! */
                result = &static_frame->body.spesh_candidates[i];
                break;
            }
        }
        if (!result) {
            if (!static_frame->body.spesh_candidates)
                static_frame->body.spesh_candidates = malloc(
                    MVM_SPESH_LIMIT * sizeof(MVMSpeshCandidate));
            result                  = &static_frame->body.spesh_candidates[num_spesh];
            result->cs              = callsite;
            result->num_guards      = num_guards;
            result->guards          = guards;
            result->bytecode        = sc->bytecode;
            result->bytecode_size   = sc->bytecode_size;
            result->handlers        = sc->handlers;
            result->num_spesh_slots = num_spesh_slots;
            result->spesh_slots     = spesh_slots;
            result->num_deopts      = num_deopts;
            result->deopts          = deopts;
            MVM_barrier();
            static_frame->body.num_spesh_candidates++;
            if (static_frame->common.header.flags & MVM_CF_SECOND_GEN)
                if (!(static_frame->common.header.flags & MVM_CF_IN_GEN2_ROOT_LIST))
                    MVM_gc_root_gen2_add(tc, (MVMCollectable *)static_frame);
            if (tc->instance->spesh_log_fh) {
                char *c_name = MVM_string_utf8_encode_C_string(tc, static_frame->body.name);
                char *c_cuid = MVM_string_utf8_encode_C_string(tc, static_frame->body.cuuid);
                fprintf(tc->instance->spesh_log_fh,
                    "Specialized '%s' (cuid: %s)\n\n", c_name, c_cuid);
                fprintf(tc->instance->spesh_log_fh,
                    "Before:\n%s\nAfter:\n%s\n\n========\n\n", before, after);
                free(before);
                free(after);
                free(c_name);
                free(c_cuid);
            }
        }
    }
    if (!result) {
        free(sc->bytecode);
        free(sc->handlers);
    }
    uv_mutex_unlock(&tc->instance->mutex_spesh_install);

    free(sc);
    return result;
}
