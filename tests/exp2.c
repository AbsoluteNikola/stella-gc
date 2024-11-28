#include "runtime.h"
#include <locale.h>

stella_object *_stella_id_helper;
stella_object *_stella_id_exp2;
stella_object *_stella_id_main;
stella_object *_stella_id__stella_cls_5(stella_object *closure, stella_object *_stella_id_r2) {;
  stella_object *_stella_id_ref; // ref
  stella_object *_stella_reg_1, *_stella_reg_2, *_stella_reg_3, *_stella_reg_4;
  gc_push_root((void**)&_stella_reg_1);
  gc_push_root((void**)&_stella_reg_2);
  gc_push_root((void**)&_stella_reg_3);
  gc_push_root((void**)&_stella_reg_4);
  #ifdef STELLA_DEBUG
  printf("[debug] enter closure _stella_id__stella_cls_5 (");
  printf("r2 = "); print_stella_object(_stella_id_r2);
  printf(") with ");
  #endif
  _stella_id_ref = STELLA_OBJECT_READ_FIELD(closure, 1);
  #ifdef STELLA_DEBUG
  printf("ref = "); print_stella_object(_stella_id_ref);
  printf("\n");
  #endif
  gc_push_root((void**)&_stella_id_r2);
  gc_push_root((void**)&_stella_id_ref);
  _stella_reg_1 = _stella_id_ref;
  _stella_reg_4 = _stella_id_ref;
  _stella_reg_3 = STELLA_OBJECT_READ_FIELD(_stella_reg_4, 0);
  _stella_reg_4 = alloc_stella_object(TAG_SUCC, 1);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_4, 0, _stella_reg_3);
  _stella_reg_2 = _stella_reg_4;
  STELLA_OBJECT_WRITE_FIELD(_stella_reg_1, 0, _stella_reg_2);
  _stella_reg_1 = &the_UNIT;
  gc_pop_root((void**)&_stella_id_ref);
  gc_pop_root((void**)&_stella_id_r2);
  gc_pop_root((void**)&_stella_reg_4);
  gc_pop_root((void**)&_stella_reg_3);
  gc_pop_root((void**)&_stella_reg_2);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object *_stella_id__stella_cls_4(stella_object *closure, stella_object *_stella_id_j) {;
  stella_object *_stella_id_ref; // ref
  stella_object *_stella_reg_1;
  gc_push_root((void**)&_stella_reg_1);
  #ifdef STELLA_DEBUG
  printf("[debug] enter closure _stella_id__stella_cls_4 (");
  printf("j = "); print_stella_object(_stella_id_j);
  printf(") with ");
  #endif
  _stella_id_ref = STELLA_OBJECT_READ_FIELD(closure, 1);
  #ifdef STELLA_DEBUG
  printf("ref = "); print_stella_object(_stella_id_ref);
  printf("\n");
  #endif
  gc_push_root((void**)&_stella_id_j);
  gc_push_root((void**)&_stella_id_ref);
  _stella_reg_1 = alloc_stella_object(TAG_FN, 2);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_1, 0, _stella_id__stella_cls_5);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_1, 1, _stella_id_ref);
  _stella_reg_1 = _stella_reg_1;
  gc_pop_root((void**)&_stella_id_ref);
  gc_pop_root((void**)&_stella_id_j);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object *_stella_id__stella_cls_3(stella_object *closure, stella_object *_stella_id_r) {;
  stella_object *_stella_id_ref; // ref
  stella_object *_stella_reg_1, *_stella_reg_2, *_stella_reg_3, *_stella_reg_4;
  gc_push_root((void**)&_stella_reg_1);
  gc_push_root((void**)&_stella_reg_2);
  gc_push_root((void**)&_stella_reg_3);
  gc_push_root((void**)&_stella_reg_4);
  #ifdef STELLA_DEBUG
  printf("[debug] enter closure _stella_id__stella_cls_3 (");
  printf("r = "); print_stella_object(_stella_id_r);
  printf(") with ");
  #endif
  _stella_id_ref = STELLA_OBJECT_READ_FIELD(closure, 1);
  #ifdef STELLA_DEBUG
  printf("ref = "); print_stella_object(_stella_id_ref);
  printf("\n");
  #endif
  gc_push_root((void**)&_stella_id_r);
  gc_push_root((void**)&_stella_id_ref);
  _stella_reg_2 = _stella_id_ref;
  _stella_reg_1 = STELLA_OBJECT_READ_FIELD(_stella_reg_2, 0);
  _stella_reg_2 = &the_UNIT;
  _stella_reg_4 = alloc_stella_object(TAG_FN, 2);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_4, 0, _stella_id__stella_cls_4);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_4, 1, _stella_id_ref);
  _stella_reg_3 = _stella_reg_4;
  _stella_reg_1 = stella_object_nat_rec(_stella_reg_1, _stella_reg_2, _stella_reg_3);
  gc_pop_root((void**)&_stella_id_ref);
  gc_pop_root((void**)&_stella_id_r);
  gc_pop_root((void**)&_stella_reg_4);
  gc_pop_root((void**)&_stella_reg_3);
  gc_pop_root((void**)&_stella_reg_2);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object *_stella_id__stella_cls_2(stella_object *closure, stella_object *_stella_id_i) {;
  stella_object *_stella_id_ref; // ref
  stella_object *_stella_reg_1;
  gc_push_root((void**)&_stella_reg_1);
  #ifdef STELLA_DEBUG
  printf("[debug] enter closure _stella_id__stella_cls_2 (");
  printf("i = "); print_stella_object(_stella_id_i);
  printf(") with ");
  #endif
  _stella_id_ref = STELLA_OBJECT_READ_FIELD(closure, 1);
  #ifdef STELLA_DEBUG
  printf("ref = "); print_stella_object(_stella_id_ref);
  printf("\n");
  #endif
  gc_push_root((void**)&_stella_id_i);
  gc_push_root((void**)&_stella_id_ref);
  _stella_reg_1 = alloc_stella_object(TAG_FN, 2);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_1, 0, _stella_id__stella_cls_3);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_1, 1, _stella_id_ref);
  _stella_reg_1 = _stella_reg_1;
  gc_pop_root((void**)&_stella_id_ref);
  gc_pop_root((void**)&_stella_id_i);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object *_stella_id__stella_cls_1(stella_object *closure, stella_object *_stella_id_n) {;
  stella_object *_stella_id_ref; // ref
  stella_object *_stella_reg_1, *_stella_reg_2, *_stella_reg_3, *_stella_reg_4, *_stella_reg_5;
  gc_push_root((void**)&_stella_reg_1);
  gc_push_root((void**)&_stella_reg_2);
  gc_push_root((void**)&_stella_reg_3);
  gc_push_root((void**)&_stella_reg_4);
  gc_push_root((void**)&_stella_reg_5);
  #ifdef STELLA_DEBUG
  printf("[debug] enter closure _stella_id__stella_cls_1 (");
  printf("n = "); print_stella_object(_stella_id_n);
  printf(") with ");
  #endif
  _stella_id_ref = STELLA_OBJECT_READ_FIELD(closure, 1);
  #ifdef STELLA_DEBUG
  printf("ref = "); print_stella_object(_stella_id_ref);
  printf("\n");
  #endif
  gc_push_root((void**)&_stella_id_n);
  gc_push_root((void**)&_stella_id_ref);
  _stella_reg_2 = _stella_id_n;
  _stella_reg_3 = &the_UNIT;
  _stella_reg_5 = alloc_stella_object(TAG_FN, 2);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_5, 0, _stella_id__stella_cls_2);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_5, 1, _stella_id_ref);
  _stella_reg_4 = _stella_reg_5;
  _stella_reg_1 = stella_object_nat_rec(_stella_reg_2, _stella_reg_3, _stella_reg_4);
  _stella_reg_2 = _stella_id_ref;
  _stella_reg_1 = STELLA_OBJECT_READ_FIELD(_stella_reg_2, 0);
  gc_pop_root((void**)&_stella_id_ref);
  gc_pop_root((void**)&_stella_id_n);
  gc_pop_root((void**)&_stella_reg_5);
  gc_pop_root((void**)&_stella_reg_4);
  gc_pop_root((void**)&_stella_reg_3);
  gc_pop_root((void**)&_stella_reg_2);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object *_fn__stella_id_helper(stella_object *_cls, stella_object *_stella_id_ref) {
  stella_object *_stella_reg_1;
  gc_push_root((void**)&_stella_reg_1);
  #ifdef STELLA_DEBUG
  printf("[debug] call function helper(");
  printf("ref = "); print_stella_object(_stella_id_ref);
  printf(")\n");
  #endif
  gc_push_root((void**)&_stella_id_ref);
  _stella_reg_1 = alloc_stella_object(TAG_FN, 2);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_1, 0, _stella_id__stella_cls_1);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_1, 1, _stella_id_ref);
  _stella_reg_1 = _stella_reg_1;
  gc_pop_root((void**)&_stella_id_ref);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object_1 _cls__stella_id_helper = { .object_header = TAG_FN, .object_fields = { &_fn__stella_id_helper } } ;
stella_object *_stella_id_helper = (stella_object *)&_cls__stella_id_helper;
stella_object *_fn__stella_id_exp2(stella_object *_cls, stella_object *_stella_id_n) {
  stella_object *_stella_reg_1, *_stella_reg_2, *_stella_reg_3, *_stella_reg_4, *_stella_reg_5;
  gc_push_root((void**)&_stella_reg_1);
  gc_push_root((void**)&_stella_reg_2);
  gc_push_root((void**)&_stella_reg_3);
  gc_push_root((void**)&_stella_reg_4);
  gc_push_root((void**)&_stella_reg_5);
  #ifdef STELLA_DEBUG
  printf("[debug] call function exp2(");
  printf("n = "); print_stella_object(_stella_id_n);
  printf(")\n");
  #endif
  gc_push_root((void**)&_stella_id_n);
  _stella_reg_3 = _stella_id_helper;
  _stella_reg_5 = alloc_stella_object(TAG_REF, 1);
  STELLA_OBJECT_INIT_FIELD(_stella_reg_5, 0, nat_to_stella_object(1));
  _stella_reg_4 = _stella_reg_5;
  _stella_reg_1 = (*(stella_object *(*)(stella_object *, stella_object *))STELLA_OBJECT_READ_FIELD(_stella_reg_3, 0))(_stella_reg_3, _stella_reg_4);
  _stella_reg_2 = _stella_id_n;
  _stella_reg_1 = (*(stella_object *(*)(stella_object *, stella_object *))STELLA_OBJECT_READ_FIELD(_stella_reg_1, 0))(_stella_reg_1, _stella_reg_2);
  gc_pop_root((void**)&_stella_id_n);
  gc_pop_root((void**)&_stella_reg_5);
  gc_pop_root((void**)&_stella_reg_4);
  gc_pop_root((void**)&_stella_reg_3);
  gc_pop_root((void**)&_stella_reg_2);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object_1 _cls__stella_id_exp2 = { .object_header = TAG_FN, .object_fields = { &_fn__stella_id_exp2 } } ;
stella_object *_stella_id_exp2 = (stella_object *)&_cls__stella_id_exp2;
stella_object *_fn__stella_id_main(stella_object *_cls, stella_object *_stella_id_n) {
  stella_object *_stella_reg_1, *_stella_reg_2;
  gc_push_root((void**)&_stella_reg_1);
  gc_push_root((void**)&_stella_reg_2);
  #ifdef STELLA_DEBUG
  printf("[debug] call function main(");
  printf("n = "); print_stella_object(_stella_id_n);
  printf(")\n");
  #endif
  gc_push_root((void**)&_stella_id_n);
  _stella_reg_1 = _stella_id_exp2;
  _stella_reg_2 = _stella_id_n;
  _stella_reg_1 = (*(stella_object *(*)(stella_object *, stella_object *))STELLA_OBJECT_READ_FIELD(_stella_reg_1, 0))(_stella_reg_1, _stella_reg_2);
  gc_pop_root((void**)&_stella_id_n);
  gc_pop_root((void**)&_stella_reg_2);
  gc_pop_root((void**)&_stella_reg_1);
  return _stella_reg_1;
}
stella_object_1 _cls__stella_id_main = { .object_header = TAG_FN, .object_fields = { &_fn__stella_id_main } } ;
stella_object *_stella_id_main = (stella_object *)&_cls__stella_id_main;

int main(int argc, char **argv) {
  int n;
  setlocale(LC_NUMERIC, "");
  scanf("%d", &n);
#ifdef STELLA_DEBUG
  printf("[debug] input n = %d\n", n);
#endif
  print_stella_object(_fn__stella_id_main(_stella_id_main, nat_to_stella_object(n))); printf("\n");
  print_stella_stats();
  print_gc_alloc_stats();
  return 0;
}