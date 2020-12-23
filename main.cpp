#include <iostream>
#include <fstream>

#define RESULT "result"

#define BLOCK_SIZE 8 + 1

#define CODE >>
#define DECODE <<
#define SHIFT_SIZE 6

#define MASK 0b00111111
#define DEMASK 0b11111100

typedef struct {
    unsigned char string[BLOCK_SIZE];
    unsigned char gamma[BLOCK_SIZE];
    size_t len;
} data_t;

void shift_enc(data_t &text, int shift) {
    unsigned char *str = text.string;
    unsigned char diff_end = (str[text.len - 1] & MASK) DECODE 8 - shift;
    for (int i = text.len - 1; i > 0; --i) {
        str[i] = (str[i] CODE shift) | (str[i - 1] & MASK) DECODE 8 - shift;
    }
    str[0] = (str[0] CODE shift) | diff_end;
}

void shift_dec(data_t &text, int shift) {
    unsigned char *str = text.string;
    unsigned char diff_end = (str[0] & DEMASK) CODE 8 - shift;
    for (int i = 0; i < text.len - 1; ++i) {
        str[i] = (str[i] DECODE shift) | (str[i + 1] & DEMASK) CODE 8 - shift;
    }
    str[text.len - 1] = (str[text.len - 1] DECODE shift) | diff_end;
}

void operator CODE(data_t &text, int shift) {
    shift_enc(text, shift);
}

void operator DECODE(data_t &text, int shift) {
    shift_dec(text, shift);
}

void gen_gamma(unsigned char *gam_buf) {
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        gam_buf[i] = static_cast<unsigned char>(rand());
    }
}

void operator^(data_t &text, unsigned char *gamma) {
    gen_gamma(gamma);
    for (int i = 0; i < text.len; ++i) {
        text.string[i] = text.string[i] ^ gamma[i];
    }
}

void code_text(std::ifstream &ifs, std::ofstream &ofs) {
    while (!ifs.eof()) {
        data_t text;
        // uchar* в char* не кастится, надо c-style
        ifs.read((char *) text.string, BLOCK_SIZE);
        text.len = ifs.gcount();
        text ^ text.gamma;
        text >> SHIFT_SIZE;
        ofs.write((char *) text.string, text.len);
    }
}

void decode_text(std::ifstream &ifs, std::ofstream &ofs) {
    while (!ifs.eof()) {
        data_t text;
        ifs.read((char *) text.string, BLOCK_SIZE);
        text.len = ifs.gcount();
        text << SHIFT_SIZE;
        text ^ text.gamma;
        ofs.write((char*)text.string, text.len);
    }
}

int main(int argc, char *argv[]) {
    std::string program_type = argv[1];

    if (program_type == "coder") {
        if (argc < 5) {
            std::cerr << "Too few args\n";
            return -1;
        }
        srand(atoi(argv[2]));


        std::ifstream fd_in(argv[3]);
        std::ofstream fd_out(argv[4], std::ios::binary);
        if (!fd_in.is_open()) {
            std::cerr << "File not opened\n";
            return -1;
        }

        code_text(fd_in, fd_out);
        fd_in.close();
        fd_out.close();
    }

    if (program_type == "decoder") {
        if (argc < 4) {
            std::cerr << "Too few args\n";
            return -1;
        }
        srand(atoi(argv[2]));


        std::ifstream input(argv[3], std::ios::binary);
        std::ofstream res(RESULT);
        if (!input.is_open() || !res.is_open()) {
            std::cerr << "File not opened\n";
            return -1;
        }

        decode_text(input, res);
        input.close();
        res.close();
    }
}
