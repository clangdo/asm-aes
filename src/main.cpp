#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <memory>

#define IN_STRING "-i"
#define IN_FILE "--if"
#define OUT_FILE "--of"

//Specified in AES
const int Nb = 4;

//aes256
const int Nk = 8;
const int Nr = 14;

char state[16];

void pad(size_t i)
{
    for(;i < 4 * Nb;i++)
    {
	state[i] = 0;
    }
}

void process_arguments(int argc, char ** argv,
		       std::shared_ptr<std::istream>& input,
		       std::shared_ptr<std::ostream>& output)
{
    for(size_t i = 0; i < argc; i++)
    {
	char* cur = argv[i];
	if(std::strcmp(cur,IN_FILE) == 0)
	{
	    input = std::make_shared<std::ifstream>(argv[++i]);
	    if(input->fail())
	    {
		std::cerr<<"Could not open file for reading: "<<argv[i]<<std::endl;
		std::exit(-1);
	    }
	}
	else if(std::strcmp(cur,OUT_FILE) == 0)
	{
	    output = std::make_shared<std::ofstream>(argv[++i]);
	    if(output->fail())
	    {
		std::cerr<<"Could not open file for writing: "<<argv[i]<<std::endl;
		std::exit(-1);
	    }
	}
	else if(std::strcmp(cur,IN_STRING) == 0)
	{
	    input = std::make_shared<std::istringstream>(argv[++i]);
	}
    }
}

int main(int argc, char ** argv)
{   
    std::shared_ptr<std::istream> input;
    std::shared_ptr<std::ostream> output;

    process_arguments(argc, argv, input, output);

    size_t state_i = 0;
    while(!input->eof())
    {
	for(state_i = 0; state_i < 4 * Nb; state_i++)
	{
	    if(input->eof())
	    {
		pad(state_i);
		break;
	    }

	    if(!input->good())
	    {
		std::cerr<<"I/O error reading file"<<std::endl;
	    }
	    
	    state[state_i] = input->get();
	}

	//transform();

	for(state_i = 0; state_i < 4 * Nb; state_i++)
	{
	    if(output->good());
	    (*output)<<state[state_i];
	}
    }

    return 0;
}
