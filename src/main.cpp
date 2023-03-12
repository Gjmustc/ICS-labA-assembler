/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 14:29:14
 * @LastEditors  : liuly
 * @LastEditTime : 2022-11-15 21:32:19
 * @Description  : A small assembler for LC-3
 */

#include "assembler.h"

bool gIsErrorLogMode = false;   //设置纠错调试模式
bool gIsHexMode = false;        //设置16进制输出模式
// A simple arguments parser
std::pair<bool, std::string> getCmdOption(char **begin, char **end,
                                          const std::string &option) {  //获取命令行指令输入
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return std::make_pair(true, *itr);
    }
    return std::make_pair(false, "");
}

bool cmdOptionExists(char **begin, char **end, const std::string &option) {  //是否输入附加命令选项-e/-s/-h
    return std::find(begin, end, option) != end;
}

int main(int argc, char **argv) {
    // Print out Basic information about the assembler
    if (cmdOptionExists(argv, argv + argc, "-h")) {
        std::cout << "This is a simple assembler for LC-3." << std::endl
                  << std::endl;
        std::cout << "\e[1mUsage\e[0m" << std::endl;
        std::cout << "./assembler \e[1m[OPTION]\e[0m ... \e[1m[FILE]\e[0m ..."
                  << std::endl
                  << std::endl;
        std::cout << "\e[1mOptions\e[0m" << std::endl;
        std::cout << "-h : print out help information" << std::endl;  //显示帮助信息
        std::cout << "-f : the path for the input file" << std::endl; //待编译文件输入路径
        std::cout << "-e : print out error information" << std::endl; //以纠错调试模式运行
        std::cout << "-o : the path for the output file" << std::endl; //编译完成文件输出路径
        std::cout << "-s : hex mode" << std::endl; //以十六进制模式转换输出
        return 0;
    }

    auto input_info = getCmdOption(argv, argv + argc, "-f");
    std::string input_filename;
    auto output_info = getCmdOption(argv, argv + argc, "-o");
    std::string output_filename;

    // Check the input file name
    if (input_info.first) {
        input_filename = input_info.second;
    } else {
        input_filename = "input.txt";
    }

    if (output_info.first) {
        output_filename = output_info.second;
    } else {
        output_filename = "";
    }

    // Check output file name
    if (output_filename.empty()) {
        output_filename = input_filename;   //若未输入output文件路径的处理方式,在input文件名后加上my后缀
        if (output_filename.find('.') == std::string::npos) {
            output_filename = output_filename + "my.bin";
        } else {
            output_filename =
                output_filename.substr(0, output_filename.rfind('.'));
            output_filename = output_filename + "my.bin";
        }
    }

    if (cmdOptionExists(argv, argv + argc, "-e")) {
        // * Error Log Mode :
        // * With error log mode, we can show error type
        SetErrorLogMode(true);
    }
    if (cmdOptionExists(argv, argv + argc, "-s")) {
        // * Hex Mode:
        // * With hex mode, the result file is shown in hex
        SetHexMode(true);
    }

    auto ass = assembler();
    auto status = ass.assemble(input_filename, output_filename); //汇编器主功能函数

    if (gIsErrorLogMode) {
        std::cout << std::dec << status << std::endl;
    }
    return 0;
}
