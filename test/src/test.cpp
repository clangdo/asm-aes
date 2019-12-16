#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>

#define TITLE_COLUMNS 30
#define RESULT_COLUMNS 10
#define DETAIL_COLUMNS 30

//Would be nice to have execution times printed.
//#define TIME_COLUMNS 

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_RESET "\x1b[0m"
#define PASSED "[" ANSI_COLOR_GREEN "Pass" ANSI_RESET "]"
#define FAILED "[" ANSI_COLOR_RED "Fail" ANSI_RESET "]"
#define DETAIL_NEWLINE std::endl<<std::setw(TITLE_COLUMNS + RESULT_COLUMNS)<<std::left<<" "

#define Nb 4

extern "C" uint8_t  aes_mul_gf28(uint8_t lhs, uint8_t rhs);
extern "C" uint32_t aes_mul_poly(uint32_t lhs, uint32_t rhs);
extern "C" void aes_shift_rows(uint8_t * state);
extern "C" void aes_mix_columns(uint8_t * state);
extern "C" void aes_sub_bytes(uint8_t * state);
extern "C" void aes_add_round_key(uint8_t * state, uint32_t * key_schedule, uint64_t offset);
extern "C" void aes_key_expand(uint32_t * key, uint32_t * key_schedule, uint64_t Nk);
extern "C" void aes_encrypt(uint8_t * state, uint32_t * key_schedule, uint64_t Nr);

const static std::string COLUMN_HEADER_TITLE("Test Name:");
const static std::string COLUMN_HEADER_RESULT("[Result]  ");
const static std::string COLUMN_HEADER_DETAIL("Details");

template <typename T>
std::string detail(T expected, T result)
{
    return std::string("\n");
}

template <>
std::string detail(std::array<uint8_t, Nb*4> expected, std::array<uint8_t, Nb*4> result)
{
    std::stringstream s;
    std::stringstream s1;
    auto i_init = std::begin(result);
    s1<<"Got: { ";
    for(auto i = i_init; i != std::end(result); i++)
    {
	if((i - i_init) % 4 == 0)
	{
	    s1<<DETAIL_NEWLINE;
	}
	s1<<std::hex<<(uint64_t)(*i)<<" ";
    }

    i_init = std::begin(expected);
    s1<<DETAIL_NEWLINE<<"}";
    s1<<DETAIL_NEWLINE<<DETAIL_NEWLINE<<"Expected: {";
    for(auto i = std::begin(expected); i != std::end(expected); i++)
    {
	if((i - i_init) % 4 == 0)
	{
	    s1<<DETAIL_NEWLINE;
	}
	s1<<std::hex<<(uint64_t)(*i)<<" ";
    }
    s1<<DETAIL_NEWLINE<<"}";
    s<<std::setw(DETAIL_COLUMNS)<<std::right<<s1.str();
    return s.str();
}

template <>
std::string detail(uint64_t expected, uint64_t result)
{
    std::stringstream s;
    std::stringstream s1;
    s1<<std::hex<<"Got: "<<result<<DETAIL_NEWLINE<<"Expected: "<<expected;
    s<<std::setw(DETAIL_COLUMNS)<<std::right<<s1.str();
    return s.str();
}

std::string title(std::string text)
{
    std::stringstream s;
    s<<std::setw(TITLE_COLUMNS)<<std::left<<text;
    return s.str();
}

template <typename T>
std::string result(bool (*test)(T, T), T expected, T result)
{
    std::stringstream s;
    if(test(expected, result))
    {
	std::stringstream s1;
	s1<<std::setw(RESULT_COLUMNS + 9)<<std::left<<PASSED;
	s<<s1.str();
    }
    else
    {
	std::stringstream s1;
	s1<<std::setw(RESULT_COLUMNS + 9)<<std::left<<FAILED;
	s<<s1.str()<<detail(expected,result);
    }

    return s.str();
}

void print_header()
{
    std::string begin_line(TITLE_COLUMNS + RESULT_COLUMNS + DETAIL_COLUMNS, '=');

    std::cout<<title(COLUMN_HEADER_TITLE)<<COLUMN_HEADER_RESULT<<COLUMN_HEADER_DETAIL<<std::endl;
    std::cout<<begin_line<<std::endl;
}

bool test_equal(uint64_t expected, uint64_t result)
{
    return (expected == result);
}

bool test_arr_equal(std::array<uint8_t, 16> expected, std::array<uint8_t, 16> result)
{
    return std::equal(std::begin(expected), std::end(expected), std::begin(result));
}

bool test_arr_equal(std::array<uint32_t, 4> expected, std::array<uint32_t, 4> result)
{
    return std::equal(std::begin(expected), std::end(expected), std::begin(result));
}

void test_gf28lib()
{
    //The below test is given in the aes specification in an example.
    uint8_t gf28t1 = 0x57; // x^6 + x^4 + x^2 + x + 1
    uint8_t gf28t2 = 0x83; // x^7 + x + 1
    uint8_t gf28r1 = aes_mul_gf28(gf28t1,gf28t2);
    std::cout<<title("Multiply GF2^8:")
	     <<result(&test_equal,
		      static_cast<uint64_t>(0xc1),
		      static_cast<uint64_t>(gf28r1))
	     <<std::endl;

    //Given in aes mix columns spec, and aes appendix B step 1
    uint32_t polyt1 = 0x305dbfd4;
    uint32_t polyt2 = 0x03010102;
    uint32_t polyr1 = aes_mul_poly(polyt1, polyt2);
    std::cout<<title("Multiply Polynomials:")
	     <<result(&test_equal,
		      static_cast<uint64_t>(0xe5816604),
		      static_cast<uint64_t>(polyr1))
	     <<std::endl;
}

std::array<uint8_t, Nb*4> swaprc(std::array<uint8_t, Nb*4> arr)
{
    for(size_t i = 0; i < 3; i++)
    {
	for(size_t j = i + 1; j < 4; j++)
	{
	    uint8_t temp = arr[i*4 + j];
	    arr[i*4 + j] = arr[j*4 + i];
	    arr[j*4 + i] = temp;
	}
    }

    return arr;
}

void test_aeslib()
{
    std::array<uint8_t, Nb*4> add_round_key_arr = swaprc({0x32,0x88,0x31,0xe0,
							  0x43,0x5a,0x31,0x37,
							  0xf6,0x30,0x98,0x07,
							  0xa8,0x8d,0xa2,0x34});

    std::array<uint8_t, Nb*4> add_round_key_expected = swaprc({0x19,0xa0,0x9a,0xe9,
							       0x3d,0xf4,0xc6,0xf8,
							       0xe3,0xe2,0x8d,0x48,
							       0xbe,0x2b,0x2a,0x08});

    std::array<uint32_t, 4> add_round_key_schedule = {0x2b7e1516,
						      0x28aed2a6,
						      0xabf71588,
						      0x09cf4f3c};
    aes_add_round_key(add_round_key_arr.data(),add_round_key_schedule.data(),0);
    std::cout<<title("Add Round Key:")<<result(&test_arr_equal, add_round_key_expected, add_round_key_arr)<<std::endl;
    
    std::array<uint8_t, Nb*4> sub_bytes_arr = swaprc({0x19,0xa0,0x9a,0xe9,
						      0x3d,0xf4,0xc6,0xf8,
						      0xe3,0xe2,0x8d,0x48,
						      0xbe,0x2b,0x2a,0x08});
    std::array<uint8_t, Nb*4> sub_bytes_expected = swaprc({0xd4,0xe0,0xb8,0x1e,
							   0x27,0xbf,0xb4,0x41,
							   0x11,0x98,0x5d,0x52,
							   0xae,0xf1,0xe5,0x30});
    aes_sub_bytes(sub_bytes_arr.data());
    std::cout<<title("Sub Bytes:")<<result(&test_arr_equal, sub_bytes_expected, sub_bytes_arr)<<std::endl;
    
    //Given in aes shift rows spec, and aes appendix B step 1
    std::array<uint8_t, Nb*4> shift_rows_arr = swaprc({0xd4,0xe0,0xb8,0x1e,
						       0x27,0xbf,0xb4,0x41,
						       0x11,0x98,0x5d,0x52,
						       0xae,0xf1,0xe5,0x30});
    std::array<uint8_t, Nb*4> shift_rows_expected = swaprc({0xd4,0xe0,0xb8,0x1e,
						      0xbf,0xb4,0x41,0x27,
						      0x5d,0x52,0x11,0x98,
						      0x30,0xae,0xf1,0xe5});
    aes_shift_rows(shift_rows_arr.data());
    std::cout<<title("Shift Rows:")<<result(&test_arr_equal, shift_rows_expected, shift_rows_arr)<<std::endl;

    //Given in appendix B step 1
    std::array<uint8_t, Nb*4> mix_columns_arr = swaprc({0xd4,0xe0,0xb8,0x1e,
							0xbf,0xb4,0x41,0x27,
							0x5d,0x52,0x11,0x98,
							0x30,0xae,0xf1,0xe5});
    std::array<uint8_t, Nb*4> mix_columns_expected = swaprc({0x04,0xe0,0x48,0x28,
							     0x66,0xcb,0xf8,0x06,
							     0x81,0x19,0xd3,0x26,
							     0xe5,0x9a,0x7a,0x4c});
    aes_mix_columns(mix_columns_arr.data());
    std::cout<<title("Mix Columns:")<<result(&test_arr_equal, mix_columns_expected, mix_columns_arr)<<std::endl;

    std::array<uint32_t, Nb> key_expand_arr = {0x2b7e1516,0x28aed2a6,
					       0xabf71588,0x09cf4f3c};
    std::array<uint32_t, Nb * (11)> key_sched;
    std::array<uint32_t, Nb> key_sched_expected = {0xd014f9a8,0xc9ee2589,
						   0xe13f0cc8,0xb6630ca6};
    aes_key_expand(key_expand_arr.data(), key_sched.data(), 4);
    std::array<uint32_t, Nb> key_sched_end;
    std::copy(key_sched.begin() + 40, key_sched.end(), key_sched_end.begin());
    std::cout<<title("Key Expansion:")<<result(&test_arr_equal,key_sched_expected,key_sched_end)<<std::endl;


    std::array<uint8_t, Nb * 4> encrypt_data = swaprc({0x32,0x88,0x31,0xe0,
						       0x43,0x5a,0x31,0x37,
						       0xf6,0x30,0x98,0x07,
						       0xa8,0x8d,0xa2,0x34});
    std::array<uint8_t, Nb * 4> encrypt_expected = swaprc({0x39,0x02,0xdc,0x19,
							   0x25,0xdc,0x11,0x6a,
							   0x84,0x09,0x85,0x0b,
							   0x1d,0xfb,0x97,0x32});

    aes_encrypt(encrypt_data.data(),key_sched.data(),10);
    std::cout<<title("Encryption:")<<result(&test_arr_equal,encrypt_data,encrypt_expected)<<std::endl;
}

int main()
{
    print_header();
    test_gf28lib();
    test_aeslib();
    return 0;
}
