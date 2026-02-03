#ifndef CREGEX_H
#define CREGEX_H

#include <stddef.h>

typedef struct cregex cregex_t;

cregex_t *cregex_compile(const char *pattern, char **err);

void cregex_free(cregex_t *r);

int cregex_match_entire(const cregex_t *r, const char *text);
int cregex_search(const cregex_t *r, const char *text);

#endif
#ifdef CREGEX_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum { SPRIT = -1, MATCH = -2, DOT = -3 };

typedef struct State State;
struct State {
    int c;
    State *out;
    State *out1;
    State *next;
};

static State *state_list = NULL;

static State *newstate(int c, State *out, State *out1) {
    State *s = (State*)malloc(sizeof(State));
    if (!s) return NULL;
    s->c = c; s->out = out; s->out1 = out1; s->next = state_list; state_list = s;
    return s;
}

typedef struct {
    State *start;
    State ***outs;
    int out_count;
} Frag;

static Frag frag_new(State *start, State ***outs, int out_count) {
    Frag f = { start, outs, out_count };
    return f;
}

static State ***outs_make(int n) {
    State ***arr = (State***)malloc(sizeof(State**) * n);
    if (!arr) return NULL;
    return arr;
}

static State ***outs_dup(State ***a, int n) {
    State ***r = (State***)malloc(sizeof(State**) * n);
    if (!r) return NULL;
    for (int i = 0; i < n; ++i) r[i] = a[i];
    return r;
}

static State ***outs_join(State ***a, int na, State ***b, int nb) {
    State ***r = (State***)malloc(sizeof(State**) * (na + nb));
    if (!r) return NULL;
    for (int i = 0; i < na; ++i) r[i] = a[i];
    for (int i = 0; i < nb; ++i) r[na + i] = b[i];
    return r;
}

static void patch(State ***outs, int n, State *s) {
    for (int i = 0; i < n; ++i) {
        *(outs[i]) = s;
    }
}

static char *infix_to_postfix(const char *in, char **err) {
    int len = strlen(in);

    char *tmp = (char*)malloc(len * 2 + 3);
    if (!tmp) { if (err) *err = strdup("malloc failed"); return NULL; }
    int j = 0;
    for (int i = 0; in[i]; ++i) {
        char c = in[i];
        if (c == '\\') {
            // escape next char
            if (!in[i+1]) break;
            tmp[j++] = in[i++]; tmp[j++] = in[i];
            continue;
        }
        tmp[j++] = c;
        char next = in[i+1];
        if (next == '\0') continue;
        int need = 0;
        if (c != '(' && c != '|' ) {
            if (next != ')' && next != '|' && next != '*') need = 1;
        }
        if (need) tmp[j++] = '&';
    }
    tmp[j] = '\0';

    char *out = (char*)malloc(j + 1);
    if (!out) { free(tmp); if (err) *err = strdup("malloc failed"); return NULL; }
    char *stack = (char*)malloc(j + 1);
    int sp = 0, op = 0;
    for (int i = 0; tmp[i]; ++i) {
        char c = tmp[i];
        if (c == '\\') { 
            out[op++] = tmp[i++]; out[op++] = tmp[i];
            continue;
        }
        if (c == '(') { stack[sp++] = c; }
        else if (c == ')') {
            while (sp && stack[sp-1] != '(') out[op++] = stack[--sp];
            if (sp == 0) { free(tmp); free(out); free(stack); if (err) *err = strdup("unmatched )"); return NULL; }
            --sp;
        } else if (c == '*' || c == '&' || c == '|') {
            int prec = (c == '*') ? 3 : (c == '&') ? 2 : 1;
            while (sp) {
                char top = stack[sp-1];
                int tprec = (top == '*') ? 3 : (top == '&') ? 2 : (top == '|') ? 1 : 0;
                if (tprec >= prec) out[op++] = stack[--sp]; else break;
            }
            stack[sp++] = c;
        } else {
            out[op++] = c;
        }
    }
    while (sp) {
        char t = stack[--sp];
        if (t == '(' || t == ')') { free(tmp); free(out); free(stack); if (err) *err = strdup("unmatched paren"); return NULL; }
        out[op++] = t;
    }
    out[op] = '\0';
    free(tmp); free(stack);
    return out;
}

static Frag frag_from_char(char c) {
    State *s = newstate((c=='.')?DOT:(unsigned char)c, NULL, NULL);

    State **outs = (State**)malloc(sizeof(State*));
    State ***outs_p = (State***)malloc(sizeof(State**));
    outs[0] = NULL;

    State **ptr = &s->out;
    *outs_p = ptr;
    Frag f = { s, (State***)malloc(sizeof(State**)), 1 };
    f.outs[0] = ptr;
    return f;
}

static Frag frag_char_literal(const char *token, int tlen) {

    char ch = token[0];
    if (ch == '\\' && tlen == 2) ch = token[1];
    State *s = newstate((ch=='.')?DOT:(unsigned char)ch, NULL, NULL);
    State **ptr = &s->out;
    State ***outs_p = (State***)malloc(sizeof(State**));
    if (!outs_p) exit(1);
    outs_p[0] = ptr;
    Frag f = { s, outs_p, 1 };
    return f;
}

static Frag build_nfa(const char *postfix, char **err) {
    int L = strlen(postfix);
    Frag *stack = (Frag*)malloc(sizeof(Frag) * (L+2));
    int sp = 0;
    for (int i = 0; i < L; ++i) {
        char c = postfix[i];
        if (c == '\\') {
            ++i;
            char lit = postfix[i];
            Frag f = frag_char_literal(&postfix[i-1], 2);
            stack[sp++] = f;
            continue;
        }
        if (c == '*') {
            if (sp == 0) { if (err) *err = strdup("bad *"); free(stack); return (Frag){0}; }
            Frag a = stack[--sp];
            State *s = newstate(SPRIT, a.start, NULL);

            patch(a.outs, a.out_count, s);

            State **ptr = &s->out1;
            State ***outs_p = (State***)malloc(sizeof(State**)); outs_p[0] = ptr;
            Frag f = { s, outs_p, 1 };
            stack[sp++] = f;
            continue;
        }
        if (c == '&') {
            if (sp < 2) { if (err) *err = strdup("bad concat"); free(stack); return (Frag){0}; }
            Frag b = stack[--sp]; Frag a = stack[--sp];
            patch(a.outs, a.out_count, b.start);
            State ***outs_p = outs_dup(b.outs, b.out_count);
            Frag f = { a.start, outs_p, b.out_count };
            stack[sp++] = f;
            continue;
        }
        if (c == '|') {
            if (sp < 2) { if (err) *err = strdup("bad |"); free(stack); return (Frag){0}; }
            Frag b = stack[--sp]; Frag a = stack[--sp];
            State *s = newstate(SPRIT, a.start, b.start);
            State ***outs_p = outs_join(a.outs, a.out_count, b.outs, b.out_count);
            Frag f = { s, outs_p, a.out_count + b.out_count };
            stack[sp++] = f;
            continue;
        }

        if (c == '.' || (unsigned char)c >= 1) {
            char token[2] = { c, 0 };
            Frag f = frag_char_literal(token, 1);
            stack[sp++] = f;
            continue;
        }
    }
    if (sp != 1) { if (err) *err = strdup("bad regex"); free(stack); return (Frag){0}; }
    Frag out = stack[0]; free(stack);

    State *m = newstate(MATCH, NULL, NULL);
    patch(out.outs, out.out_count, m);
    return out;
}

/* Epsilon-closure */
typedef struct SList { State **s; int n; } SList;

static void addstate(State **list, int *n, State *s) {
    if (!s) return;
    for (int i = 0; i < *n; ++i) if (list[i] == s) return;
    list[(*n)++] = s;
}

static SList eclosure(State **states, int n) {
    State **stack = (State**)malloc(sizeof(State*) * 1024);
    int sp = 0;
    State **out = (State**)malloc(sizeof(State*) * 1024);
    int on = 0;
    for (int i = 0; i < n; ++i) { stack[sp++] = states[i]; }
    while (sp) {
        State *s = stack[--sp];
        if (!s) continue;

        if (s->c == SPRIT) {
            if (s->out) stack[sp++] = s->out;
            if (s->out1) stack[sp++] = s->out1;
        } else {
            addstate(out, &on, s);
        }
    }

    SList r = { out, on };
    free(stack);
    return r;
}

static SList move_states(State **states, int n, unsigned char ch) {
    State **out = (State**)malloc(sizeof(State*) * 1024);
    int on = 0;
    for (int i = 0; i < n; ++i) {
        State *s = states[i];
        if (!s) continue;
        if (s->c == (int)ch || s->c == DOT) {
            if (s->out) addstate(out, &on, s->out);
        }
    }
    SList r = { out, on };
    return r;
}

typedef struct {
    int *trans;
    int accept;
    State **nfastates;
    int n_nfa;
} DState;

struct cregex {
    DState *states;
    int nstates;
    int start;
};

static int find_dstate(DState *dstates, int nd, State **nfastates, int nnfa) {
    for (int i = 0; i < nd; ++i) {
        if (dstates[i].n_nfa != nnfa) continue;
        int match = 1;
        for (int j = 0; j < nnfa; ++j) if (dstates[i].nfastates[j] != nfastates[j]) { match = 0; break; }
        if (match) return i;
    }
    return -1;
}

cregex_t *cregex_compile(const char *pattern, char **err) {
    if (!pattern) { if (err) *err = strdup("null pattern"); return NULL; }
    state_list = NULL;
    char *post = infix_to_postfix(pattern, err);
    if (!post) return NULL;
    Frag nfa = build_nfa(post, err);
    free(post);
    if (!nfa.start) { if (err && !*err) *err = strdup("failed to build nfa"); return NULL; }

    State *init_arr[1] = { nfa.start };
    SList s0 = eclosure(init_arr, 1);

    DState *dstates = NULL; int nd = 0; int cap = 0;

    auto_add:
    ;
    if (nd + 1 > cap) {
        int nc = cap ? cap * 2 : 16;
        DState *tmp = (DState*)realloc(dstates, sizeof(DState) * nc);
        if (!tmp) { if (err) *err = strdup("malloc failed"); return NULL; }
        dstates = tmp; cap = nc;
    }

    State **arr0 = s0.s; int n0 = s0.n;

    for (int i = 1; i < n0; ++i) {
        State *v = arr0[i]; int j = i - 1; while (j >= 0 && arr0[j] > v) { arr0[j+1] = arr0[j]; j--; } arr0[j+1] = v;
    }
    dstates[nd].nfastates = arr0; dstates[nd].n_nfa = n0;
    dstates[nd].trans = (int*)malloc(sizeof(int) * 256);
    for (int i = 0; i < 256; ++i) dstates[nd].trans[i] = -1;
    dstates[nd].accept = 0;
    for (int i = 0; i < n0; ++i) if (arr0[i]->c == MATCH) dstates[nd].accept = 1;
    nd++;

    for (int idx = 0; idx < nd; ++idx) {

        State **src = dstates[idx].nfastates; int sn = dstates[idx].n_nfa;
        for (int ch = 0; ch < 256; ++ch) {
            SList moved = move_states(src, sn, (unsigned char)ch);
            if (moved.n == 0) { free(moved.s); continue; }
            SList closure = eclosure(moved.s, moved.n);
            free(moved.s);

            for (int i = 1; i < closure.n; ++i) {
                State *v = closure.s[i]; int j = i - 1; while (j >= 0 && closure.s[j] > v) { closure.s[j+1] = closure.s[j]; j--; } closure.s[j+1] = v;
            }
            int found = find_dstate(dstates, nd, closure.s, closure.n);
            if (found == -1) {
                if (nd + 1 > cap) {
                    int nc = cap ? cap * 2 : 16;
                    DState *tmp = (DState*)realloc(dstates, sizeof(DState) * nc);
                    if (!tmp) { if (err) *err = strdup("malloc failed"); return NULL; }
                    dstates = tmp; cap = nc;
                }
                dstates[nd].nfastates = closure.s; dstates[nd].n_nfa = closure.n;
                dstates[nd].trans = (int*)malloc(sizeof(int) * 256);
                for (int i = 0; i < 256; ++i) dstates[nd].trans[i] = -1;
                dstates[nd].accept = 0;
                for (int i = 0; i < closure.n; ++i) if (closure.s[i]->c == MATCH) dstates[nd].accept = 1;
                dstates[idx].trans[ch] = nd;
                nd++;
            } else {
                dstates[idx].trans[ch] = found;
                free(closure.s);
            }
        }
    }

    cregex_t *r = (cregex_t*)malloc(sizeof(cregex_t));
    r->states = dstates; r->nstates = nd; r->start = 0;

    return r;
}

void cregex_free(cregex_t *r) {
    if (!r) return;
    for (int i = 0; i < r->nstates; ++i) {
        free(r->states[i].trans);
        free(r->states[i].nfastates);
    }
    free(r->states);

    State *s = state_list;
    while (s) { State *n = s->next; free(s); s = n; }
    free(r);
}

int cregex_match_entire(const cregex_t *r, const char *text) {
    if (!r || !text) return 0;
    int cur = r->start;
    for (const unsigned char *p = (const unsigned char*)text; *p; ++p) {
        int nx = r->states[cur].trans[*p];
        if (nx == -1) return 0;
        cur = nx;
    }
    return r->states[cur].accept;
}

int cregex_search(const cregex_t *r, const char *text) {
    if (!r || !text) return 0;
    size_t L = strlen(text);
    for (size_t i = 0; i < L; ++i) {
        int cur = r->start;
        for (size_t j = i; j < L; ++j) {
            int nx = r->states[cur].trans[(unsigned char)text[j]];
            if (nx == -1) break;
            cur = nx;
            if (r->states[cur].accept) return 1;
        }
    }
    return 0;
}

#endif