import sys

def get_bpadding(padding):
    if padding == 1:
        return '{0:01b}'.format(0)
    if padding == 2:
        return '{0:02b}'.format(0)
    if padding == 3:
        return '{0:03b}'.format(0)
    if padding == 4:
        return '{0:04b}'.format(0)
    if padding == 5:
        return '{0:05b}'.format(0)
    if padding == 6:
        return '{0:06b}'.format(0)
    if padding == 7:
        return '{0:07b}'.format(0)

def get_addr(func_str, arg_type, arg):
    if arg_type == "VAL":
        func_str += '{0:08b}'.format(int(arg))
        func_str += '{0:02b}'.format(0)
    if arg_type == "REG":
        func_str += '{0:03b}'.format(int(arg))
        func_str += '{0:02b}'.format(1)
        

    if arg_type == "STK" or arg_type == "PTR":
        symbols = "ABCDEFGHIJKLMNOPQRSTUVWXYZabdef"
        i = 0
        while i < 32:
            if arg == symbols[i]:
                arg = int(i)
                break
            i += 1

        if arg_type == "STK":
            func_str += '{0:05b}'.format(int(arg))
            func_str += '{0:02b}'.format(2)
        if arg_type == "PTR":
            func_str += '{0:05b}'.format(int(arg))
            func_str += '{0:02b}'.format(3)
            
    return func_str


src = sys.argv[1]
dest = sys.argv[2]

fr = open(src, "r");
lines = fr.readlines()

prog_str = ""
num_of_instr = 0
i = 0
while i < len(lines):
    func_str = ""
    
    instruction = lines[i].split(" ");
    if len(instruction) == 3:
        func_label = int(instruction[2]);
        prog_str += '{0:03b}'.format(func_label)
    else:
        num_of_instr += 1;
        opcode = instruction[4].strip("\n")

        if opcode == "MOV" or opcode == "REF" or opcode == "ADD": # Add source address
            type2 = instruction[7]
            arg2 = instruction[8].strip("\n");
            func_str = get_addr(func_str, type2, arg2)

        if opcode != "RET":
            if len(instruction) > 6:
                type1 = instruction[5]
                arg1 = instruction[6].strip("\n")
                func_str = get_addr(func_str, type1, arg1) # Add destination

        if opcode == "MOV":
            func_str += '{0:03b}'.format(0)
        if opcode == "CAL":
            func_str += '{0:03b}'.format(1)
        if opcode == "RET":
            func_str += '{0:03b}'.format(2)
            if i == len(lines)-1 or lines[i+1].split(" ")[0] == "FUNC":
                func_str += '{0:05b}'.format(num_of_instr)
                num_of_instr = 0;
        if opcode == "REF":
            func_str += '{0:03b}'.format(3)
        if opcode == "ADD":
            func_str += '{0:03b}'.format(4)
        if opcode == "PRINT":
            func_str += '{0:03b}'.format(5)
        if opcode == "NOT":
            func_str += '{0:03b}'.format(6)
        if opcode == "EQU":
            func_str += '{0:03b}'.format(7)

    prog_str += func_str
    i += 1

if sys.argv[1] == "4_noRet.asm":
    prog_str += '{0:05b}'.format(1)


padding = 8 - len(prog_str)%8
if padding != 8:
    prog_str = get_bpadding(padding) + prog_str

prog_str = '%0*x' % ((len(prog_str) + 3) // 4, int(prog_str, 2))

fw = open(dest, "wb")
fw.write(bytearray.fromhex(prog_str))
