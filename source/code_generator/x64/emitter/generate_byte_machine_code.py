from keystone import *
from itertools import product

def compile_asm(asm_code, arch, mode):
    try:
        ks = Ks(arch, mode)
        encoding, _ = ks.asm(asm_code)
        formatted_encoding = ', '.join('0x{:02x}'.format(byte) for byte in encoding)
        return formatted_encoding, encoding
    except KsError as e:
        print(f"ERROR: {e}")
        return None, None

def generate_mov_combinations(registers, arch, mode):
    combinations = []
    for reg1, reg2 in product(registers, repeat=2):
        asm_code = f"MOV {reg1}, {reg2}"
        hex_code, _ = compile_asm(asm_code, arch, mode)
        if hex_code:
            entry = f"{{{{ x64_operation::mov, {{ x64_operand::{reg1.lower()}, x64_operand::{reg2.lower()} }} }}, {{ {hex_code} }} }}"
            combinations.append(entry)

    # generate MOVs to every register where the source is an imm64
    for reg in registers:
        asm_code = f"MOV {reg}, 0x123456789ABCDEF0"  # Example imm64 value
        hex_code, encoding = compile_asm(asm_code, arch, mode)
        if hex_code and encoding:
            # keep only the first 2 bytes
            first_two_bytes = ', '.join('0x{:02x}'.format(byte) for byte in encoding[:2])
            entry = f"{{{{ x64_operation::mov, {{ x64_operand::{reg.lower()}, x64_operand::imm64 }} }}, {{ {first_two_bytes} }} }}"
            combinations.append(entry)

    return combinations

if __name__ == "__main__":
    registers = ["RAX", "RBX", "RCX", "RDX", "RSI", "RDI", "RBP", "RSP", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15"]
    arch = KS_ARCH_X86
    mode = KS_MODE_64

    combinations = generate_mov_combinations(registers, arch, mode)

    with open("x64_mov_combinations.txt", "w") as f:
        for entry in combinations:
            f.write(f"{entry},\n")

    print("Hex codes for MOV operations have been saved to 'x64_mov_combinations.txt'.")