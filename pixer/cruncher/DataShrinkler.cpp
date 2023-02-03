// copied/modified
#ifndef SHRINKLER_TITLE
#define SHRINKLER_TITLE ("Shrinkler executable file compressor by Blueberry - development version (built " __DATE__ " " __TIME__ ")\n\n")
#endif

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <sys/stat.h>

using std::string;

//#include "HunkFile.h"
#define NUM_RELOC_CONTEXTS 256

#include "LZDecoder.h"
#include "DataFile.h"


int compressData(const vector<uint8_t> &vdata, vector<uint8_t> & compressed)
{
	DataFile* orig = new DataFile;
	orig->loadbin(vdata);
// min, max, default
    int references=10000;

  /*

	FlagParameter   data          ("-d", "--data",                                 argc, argv, consumed);
	FlagParameter   bytes         ("-b", "--bytes",                                argc, argv, consumed);
	FlagParameter   header        ("-w", "--header",                               argc, argv, consumed);
	FlagParameter   hunkmerge     ("-h", "--hunkmerge",                            argc, argv, consumed);
	FlagParameter   no_crunch     ("-u", "--no-crunch",                            argc, argv, consumed);
	FlagParameter   overlap       ("-o", "--overlap",                              argc, argv, consumed);
	FlagParameter   mini          ("-m", "--mini",                                 argc, argv, consumed);
	FlagParameter   commandline   ("-c", "--commandline",                          argc, argv, consumed);
	IntParameter    iterations    ("-i", "--iterations",      1,        9,    1*p, argc, argv, consumed);
	IntParameter    length_margin ("-l", "--length-margin",   0,      100,    1*p, argc, argv, consumed);
	IntParameter    same_length   ("-a", "--same-length",     1,   100000,   10*p, argc, argv, consumed);
	IntParameter    effort        ("-e", "--effort",          0,   100000,  100*p, argc, argv, consumed);
	IntParameter    skip_length   ("-s", "--skip-length",     2,   100000, 1000*p, argc, argv, consumed);
	IntParameter    references    ("-r", "--references",   1000,100000000, 100000, argc, argv, consumed);
	StringParameter text          ("-t", "--text",                                 argc, argv, consumed);
	StringParameter textfile      ("-T", "--textfile",                             argc, argv, consumed);
	HexParameter    flash         ("-f", "--flash",                             0, argc, argv, consumed);
	FlagParameter   no_progress   ("-p", "--no-progress",                          argc, argv, consumed);

*/
	PackParams params;
	params.parity_context = false;//!bytes.seen;
	params.iterations = 1;// iterations.value;
	params.length_margin = 1;// length_margin.value;
	params.skip_length = 2; //skip_length.value;
	params.match_patience = 100;// effort.value;
	params.max_same_length = 1;// same_length.value;


	printf("Crunching...\n\n");
	RefEdgeFactory edge_factory(references);
	DataFile* crunched = orig->crunch(&params, &edge_factory, /*!no_progress.seen*/true);
	delete orig;
	printf("References considered:%8d\n", edge_factory.max_edge_count);
	printf("References discarded:%9d\n\n", edge_factory.max_cleaned_edges);

	//printf("Saving file %s...\n\n", outfile);
	//crunched->save(outfile, header.seen);

	crunched->saveBinNoHeader(compressed);
	printf("Final file size: %d\n\n", (int)compressed.size());
	delete crunched;

	if (edge_factory.max_edge_count > references) {
		printf("Note: compression may benefit from a larger reference buffer (-r option).\n\n");
	}

	return 0;
}
