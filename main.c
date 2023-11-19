// #include "tb/include/tb.h"
#define CUIK_USE_TB

int main() {
// 	TB_FeatureSet features = { 0 };
// 	TB_Module* module = tb_module_create_for_host(&features, false);
//     TB_Function* function = tb_function_create(module, 4, "add", TB_LINKAGE_PRIVATE, TB_COMDAT_NONE);
//     TB_PrototypeParam ret = { TB_TYPE_I32 };
//     TB_FunctionPrototype* prototype = tb_prototype_create(module, TB_STDCALL, 0, NULL, 1, &ret, false);
//     tb_function_set_prototype(function, prototype, NULL);

//     TB_Node* temporary_a = tb_inst_local(function, 4, 4);
//     TB_Node* temporary_b = tb_inst_local(function, 4, 4);

//     TB_Node* integer_a = tb_inst_sint(function, TB_TYPE_I32, 100);
//     TB_Node* integer_b = tb_inst_sint(function, TB_TYPE_I32, 200);
//     tb_inst_store(function, TB_TYPE_I32, temporary_a, integer_a, 4, false);
//     tb_inst_store(function, TB_TYPE_I32, temporary_b, integer_b, 4, false);

//     TB_Node* add = tb_inst_add(function, temporary_a, temporary_b, TB_ARITHMATIC_NONE);

//     tb_inst_ret(function, 1, &add);
//     tb_function_print(function, tb_default_print_callback, stdout);

//     TB_Passes* p = tb_pass_enter(function, tb_function_get_arena(function));

//     TB_FunctionOutput* out = tb_pass_codegen(p, true);

//    // tb_output_print_asm(out, stdout);

//     size_t code_length = 0;
//     uint8_t *received_code = tb_output_get_code(out, &code_length);

//     // Print individual bytes
//     printf("Received code bytes: ");
//     for (size_t i = 0; i < code_length; ++i) {
//         printf("%02x ", received_code[i]);
//     }
//     printf("\n");


    return 0;
}