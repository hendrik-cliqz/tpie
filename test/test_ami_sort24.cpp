// Copyright (c) 1995 Darren Vengroff
//
// File: test_ami_sort24.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/24/95
//

#include <versions.h>
VERSION(test_ami_sort24_cpp,"$Id: test_ami_sort24.cpp,v 1.8 2003-04-20 23:51:40 tavi Exp $");

#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
// For cout.
#include <iostream>
// For ofstream.
#include <fstream>

using std::cout;
using std::ofstream;

// Get information on the configuration to test.
#include "app_config.h"
#include "parse_args.h"

// Define it all.
#include <ami.h>

#include <ami_kb_sort.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>


#include <cpu_timer.h>

// This is the type of object we will sort.

union sort_obj
{
    kb_key key_val;
    char filler[24];

    // How to extract the key for key bucket sorting.
    inline operator kb_key(void) const
    {
        return key_val;
    }
};



// A scan object to generate random keys.
class scan_random_so : AMI_scan_object {
private:
    unsigned int max, remaining;
public:
    scan_random_so(unsigned int count = 1000, int seed = 17);
    virtual ~scan_random_so(void);
    AMI_err initialize(void);
    AMI_err operate(sort_obj *out1, AMI_SCAN_FLAG *sf);
};


scan_random_so::scan_random_so(unsigned int count, int seed) :
max(count), remaining(count)
{
    LOG_APP_DEBUG("scan_random_so seed = ");
    LOG_APP_DEBUG(seed);
    LOG_APP_DEBUG('\n');

    srandom(seed);
}

scan_random_so::~scan_random_so(void)
{
}


AMI_err scan_random_so::initialize(void)
{
    remaining = max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err scan_random_so::operate(sort_obj *out1, AMI_SCAN_FLAG *sf)
{
    if ((*sf = remaining--)) {
        out1->key_val = random();
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};



static char def_srf[] = "/var/tmp/oss.txt";
static char def_rrf[] = "/var/tmp/osr.txt";

static char *sorted_results_filename = def_srf;
static char *rand_results_filename = def_rrf;

static bool report_results_random = false;
static bool report_results_sorted = false;

static bool kb_sort = false;

static const char as_opts[] = "R:S:rsk";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'R':
            rand_results_filename = optarg;
        case 'r':
            report_results_random = true;
            break;
        case 'S':
            sorted_results_filename = optarg;
        case 's':
            report_results_sorted = true;
            break;
        case 'k':
            kb_sort = !kb_sort;
            break;
    }
}

#include <signal.h>

int main(int argc, char **argv)
{

#if 0    
    {
        // What's going on with SIGIO?
        int sa;

        struct sigaction old_action;

        sa = sigaction(SIGIO, NULL, &old_action);

        cout << sa;
        
    }
#endif
    
    AMI_err ae;

    cpu_timer cput;
    
    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed << ' ';
    }
    
    // Set the amount of main memory:
    MM_manager.set_memory_limit (test_mm_size);
        
    AMI_STREAM<sort_obj> amis0;
    AMI_STREAM<sort_obj> amis1;
        
    // Write some ints.
    scan_random_so rnds(test_size,random_seed);
    
    ae = AMI_scan(&rnds, &amis0);

    if (verbose) {
        cout << "Wrote the random values.\n";
        cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    // Streams for reporting random vand/or sorted values to ascii
    // streams.
    
    ofstream *oss;
    cxx_ostream_scan<sort_obj> *rpts = NULL;
    ofstream *osr;
    cxx_ostream_scan<sort_obj> *rptr = NULL;
    
    if (report_results_random) {
        osr = new ofstream(rand_results_filename);
        rptr = new cxx_ostream_scan<sort_obj>(osr);
    }
    
    if (report_results_sorted) {
        oss = new ofstream(sorted_results_filename);
        rpts = new cxx_ostream_scan<sort_obj>(oss);
    }
    
    if (report_results_random) {
        ae = AMI_scan(&amis0, rptr);
    }

    // Make the input stream read-once.

    amis0.persist(PERSIST_READ_ONCE);
    
    cput.reset();
    cput.start();

    if (kb_sort) {
        key_range range(KEY_MIN, KEY_MAX);
        ae = AMI_kb_sort(amis0, amis1, range);
    } else {
        ae = AMI_sort(&amis0, &amis1);
    }

    cput.stop();

#if 0    
    // Make the input stream persist, so we can see how many blocks it
    // contains when the program exits.

    amis0.persist(PERSIST_PERSISTENT);
#endif
    
    if (verbose) {
        cout << "Sorted them.\n";
        cout << "ae = " << ae << '\n';
        cout << "Sorted stream length = " << amis1.stream_len() << '\n';
    }

    cout << cput;
    
    if (report_results_sorted) {
        ae = AMI_scan(&amis1, rpts);
    }
    cout << '\n';
    
    return 0;
}