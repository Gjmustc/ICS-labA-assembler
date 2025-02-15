/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 15:10:31
 * @LastEditors  : liuly
 * @LastEditTime : 2022-11-15 21:10:23
 * @Description  : content for samll assembler
 */

#include "assembler.h"
#include <string>

// add label and its address to symbol table
void LabelMapType::AddLabel(const std::string &str, const unsigned address) {
    labels_.insert({str, address});
}
// locate the address of label in symbol table
int LabelMapType::GetAddress(const std::string &str) const {
    if (labels_.find(str) == labels_.end()) {
        // not found
        return -1;
    }
    return labels_.at(str);//at()和find()都是通过key在unordered_map哈希表中根据key查找元素，at返回的是元素value，find返回的是迭代器
}

std::string assembler::TranslateOprand(unsigned int current_address, std::string str, int opcode_length) {
    // Translate the oprand
    str = Trim(str);
    auto item = label_map.GetAddress(str);
    if (item != -1) { //操作数是标签
        // str is a label
        // TO BE DONE
        item = item - current_address - 1;  //PCoffset
        std::string addr = NumberToAssemble(item);
        //printf("label :%d\n",opcode_length);
        addr.erase(0,16-opcode_length);
        return addr;
    }
    if (str[0] == 'R') { //操作数是寄存器
        // str is a register
        // TO BE DONE
        std::string reg = NumberToAssemble(str[1]-'0');
        //printf("reg :%d\n",opcode_length);
        reg.erase(0,13);
        return reg;
    } 
    else {  //操作数是立即数
        // str is an immediate number
        // TO BE DONE
        std::string imm = NumberToAssemble(RecognizeNumberValue(str));
        //printf("imm :%d\n",opcode_length);
        imm.erase(0,16-opcode_length);
        return imm;
    }
}

std::string assembler::LineLabelSplit(const std::string &line, int current_address) {
    // split the label
    auto first_whitespace_position = line.find(' ');
    auto first_token = line.substr(0, first_whitespace_position);

    if (IsLC3Pseudo(first_token) == -1 && IsLC3Command(first_token) == -1 &&
        IsLC3TrapRoutine(first_token) == -1) {
        // * This is a label
        // save it in label_map
        // TO BE DONE
        label_map.AddLabel(first_token,current_address);
        // remove label from the line
        if (first_whitespace_position == std::string::npos) { //该行只包含标签
            // nothing else in the line
            return "";
        }
        auto command = line.substr(first_whitespace_position + 1);
        return Trim(command);  //返回去除标签项后的指令行
    }
    return line;
}

// Scan #1: save commands and labels with their addresses
int assembler::firstPass(std::string &input_filename) {
    std::string line;
    std::ifstream input_file(input_filename);
    if (!input_file.is_open()) {
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }

    int orig_address = -1;
    int current_address = -1;

    while (std::getline(input_file, line)) { //逐行读取文件

        line = FormatLine(line);
        if (line.empty()) {
            continue;
        }

        auto command = LineLabelSplit(line, current_address);
        if (command.empty()) {
            continue;
        }

        // OPERATION or PSEUDO?
        auto first_whitespace_position = command.find(' ');
        auto first_token = command.substr(0, first_whitespace_position);

        // Special judge .ORIG and .END
        if (first_token == ".ORIG") {
            std::string orig_value =
                command.substr(first_whitespace_position + 1);
            orig_address = RecognizeNumberValue(orig_value);
            if (orig_address == std::numeric_limits<int>::max()) {
                // @ Error orig address
                return -2;
            }
            current_address = orig_address;  //代码起始地址赋值
            continue;
        }

        if (orig_address == -1) { //orig地址未被正确初始化，非正规程序
            // @ Error Program begins before .ORIG
            return -3;
        }

        if (first_token == ".END") {  //读取到.END即终止汇编过程
            break;
        }
        //逐一保存指令及对应内存地址
        // For LC3 Operation
        if (IsLC3Command(first_token) != -1 ||
            IsLC3TrapRoutine(first_token) != -1) {
            commands.push_back(
                {current_address, command, CommandType::OPERATION});  
            current_address += 1;
            continue;
        }

        // For Pseudo code
        commands.push_back({current_address, command, CommandType::PSEUDO});
        auto operand = command.substr(first_whitespace_position + 1);
        if (first_token == ".FILL") {
            auto num_temp = RecognizeNumberValue(operand);
            if (num_temp == std::numeric_limits<int>::max()) {
                // @ Error Invalid Number input @ FILL
                return -4;
            }
            if (num_temp > 65535 || num_temp < -65536) {
                // @ Error Too large or too small value  @ FILL
                return -5;
            }
            current_address += 1;
        }
        if (first_token == ".BLKW") {
            // modify current_address
            // TO BE DONE
            auto num_temp = RecognizeNumberValue(operand);
            if (num_temp == std::numeric_limits<int>::max()) {
                // @ Error Invalid Number input @ BLKW
                return -6;
            }
            if (num_temp > 100 || num_temp <= 0) {
                // @ Error Too large or too small value  @ BLKW
                return -7;
            }
            current_address += (num_temp-1);
        }
        if (first_token == ".STRINGZ") {
            // modify current_address
            // TO BE DONE
            current_address += (int)operand.length();
        }
    }
    // OK flag
    return 0;
}

//逐一转译伪指令或指令或陷入服务程序

std::string assembler::TranslatePseudo(std::stringstream &command_stream) {
    std::string pseudo_opcode;
    std::string output_line;
    command_stream >> pseudo_opcode;
    if (pseudo_opcode == ".FILL") {
        std::string number_str;
        command_stream >> number_str;
        output_line = NumberToAssemble(RecognizeNumberValue(number_str));
        if (gIsHexMode)
            output_line = ConvertBin2Hex(output_line);
    } 
    else if (pseudo_opcode == ".BLKW") {
        // Fill 0 here
        // TO BE DONE
        std::string number_str;
        command_stream >> number_str;
        int i = RecognizeNumberValue(number_str);
        while(i>0)
        {
            if(i==1) 
            {
                output_line += "0000000000000000";
                break;
            }
            output_line += "0000000000000000\n";
            i--;
        }
    } 
    else if (pseudo_opcode == ".STRINGZ") {
        // Fill string here
        // TO BE DONE
        std::string char_str;
        command_stream >> char_str;
        int i = (int)char_str.length();
        int j = 0;
        while(i > 0)
        {
            if(char_str[j] == '"')
            {
                i--,j++;
                continue;
            }
            output_line += NumberToAssemble(char_str[j]);
            output_line += "\n";
            i--,j++;
        }
        output_line += "0000000000000000";   //'\0'
    }
    return output_line;
}

std::string assembler::TranslateCommand(std::stringstream &command_stream, unsigned int current_address) {
    std::string opcode;
    command_stream >> opcode;
    auto command_tag = IsLC3Command(opcode);

    std::vector<std::string> operand_list;   //操作数向量
    std::string operand;
    while (command_stream >> operand) {
        operand_list.push_back(operand);
    }
    auto operand_list_size = operand_list.size();

    std::string output_line;

    if (command_tag == -1) {
        // This is a trap routine
        command_tag = IsLC3TrapRoutine(opcode);      //根据陷入矢量表下标查找对应机器码
        output_line = kLC3TrapMachineCode[command_tag];
    } 
    else {
        // This is a LC3 command
        switch (command_tag) {
        case 0:
            // "ADD"
            output_line += "0001";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            if (operand_list[2][0] == 'R') {
                // The third operand is a register
                output_line += "000";
                output_line +=
                    TranslateOprand(current_address, operand_list[2]);
            } else {
                // The third operand is an immediate number
                output_line += "1";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], 5);
            }
            break;
        case 1:
            // "AND"
            // TO BE DONE
            output_line += "0101";
            if(operand_list_size != 3){
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            if (operand_list[2][0] == 'R') {
                // The third operand is a register
                output_line += "000";
                output_line +=
                    TranslateOprand(current_address, operand_list[2]);
            } else {
                // The third operand is an immediate number
                output_line += "1";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], 5);
            }
            break;
        case 2:
            // "BR"
            // TO BE DONE
            output_line += "0000111";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 3:
            // "BRN"
            // TO BE DONE
            output_line += "0000100";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 4:
            // "BRZ"
            // TO BE DONE
            output_line += "0000010";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 5:
            // "BRP"
            // TO BE DONE
            output_line += "0000001";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 6:
            // "BRNZ"
            // TO BE DONE
            output_line += "0000110";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 7:
            // "BRNP"
            // TO BE DONE
            output_line += "0000101";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 8:
            // "BRZP"
            // TO BE DONE
            output_line += "0000011";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],9);
            break;
        case 9:
            // "BRNZP"
            output_line += "0000111";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 10:
            // "JMP"
            // TO BE DONE
            output_line += "1100000";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0]);
            output_line += "000000";
            break;
        case 11:
            // "JSR"
            // TO BE DONE
            output_line += "01001";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address,operand_list[0],11);
            break;
        case 12:
            // "JSRR"
            // TO BE DONE
            output_line += "0100000";
            if(operand_list_size != 1){
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += "000000";
            break;
        case 13:
            // "LD"
            // TO BE DONE
            output_line += "0010";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 14:
            // "LDI"
            // TO BE DONE
            output_line += "1010";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 15:
            // "LDR"
            // TO BE DONE
            output_line += "0110";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += TranslateOprand(current_address, operand_list[2],6);
            break;
        case 16:
            // "LEA"
            // TO BE DONE
            output_line += "1110";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 17:
            // "NOT"
            // TO BE DONE
            output_line += "1001";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += "111111";
            break;
        case 18:
            // RET
            output_line += "1100000111000000";
            if (operand_list_size != 0) {
                // @ Error operand numbers
                exit(-30);
            }
            break;
        case 19:
            // RTI
            // TO BE DONE
            output_line += "1000000000000000";
            if (operand_list_size != 0) {
                // @ Error operand numbers
                exit(-30);
            }
            break;
        case 20:
            // ST
            output_line += "0011";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 21:
            // STI
            // TO BE DONE
            output_line += "1011";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 22:
            // STR
            // TO BE DONE
            output_line += "0111";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += TranslateOprand(current_address, operand_list[2],6);
            break;
        case 23:
            // TRAP
            // TO BE DONE
            output_line += "11110000";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0],8);
            break;
        default:
            // Unknown opcode
            // @ Error
            output_line += "1111111111111111";
            break;
        }
    }

    if (gIsHexMode)
        output_line = ConvertBin2Hex(output_line);

    return output_line;
}

int assembler::secondPass(std::string &output_filename) {
    // Scan #2:
    // Translate
    std::ofstream output_file;
    // Create the output file
    output_file.open(output_filename);
    if (!output_file) {
        // @ Error at output file
        return -20;
    }

    for (const auto &command : commands) {  //从逐行保存的指令中依次读取内存地址、指令内容、指令类型
        const unsigned address = std::get<0>(command);
        const std::string command_content = std::get<1>(command);
        const CommandType command_type = std::get<2>(command);
        auto command_stream = std::stringstream(command_content);

        if (command_type == CommandType::PSEUDO) {
            // Pseudo
            output_file << TranslatePseudo(command_stream) << std::endl;
        } else {
            // LC3 command
            output_file << TranslateCommand(command_stream, address) << std::endl;
        }
    }

    // Close the output file
    output_file.close();
    // OK flag
    return 0;
}

// 汇编主功能函数定义——两次扫描若正确则返回0，否则返回对应错误码
int assembler::assemble(std::string &input_filename, std::string &output_filename) {
    auto first_scan_status = firstPass(input_filename);
    if (first_scan_status != 0) {  
        return first_scan_status;
    }
    auto second_scan_status = secondPass(output_filename);
    if (second_scan_status != 0) {
        return second_scan_status;
    }
    // OK flag
    return 0;
}
