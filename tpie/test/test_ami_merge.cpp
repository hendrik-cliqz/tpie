// Copyright (c) 1994 Darren Erik Vengroff
//
// File: test_ami_merge.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 6/2/94
//

static char test_ami_merge_id[] = "$Id: test_ami_merge.cpp,v 1.7 1995-06-20 20:15:48 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// refereneced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___test_ami_merge_id_compiler_fooler {
    char *pc;
    ___test_ami_merge_id_compiler_fooler *next;
} the___test_ami_merge_id_compiler_fooler = {
    test_ami_merge_id,
    &the___test_ami_merge_id_compiler_fooler
};

#include <iostream.h>
#include <fstream.h>

#include "app_config.h"        
#include "parse_args.h"

// Define it all.
#include <ami.h>

// Utitlities for ascii output.
#include <ami_scan_utils.h>

// Get some scanners and a merger.

#include "scan_square.h"
#include "scan_count.h"

#include "merge_interleave.h"

static char def_crf[] = "/var/tmp/osc.txt";
static char def_irf[] = "/var/tmp/osi.txt";
static char def_frf[] = "/var/tmp/osf.txt";

static char *count_results_filename = def_crf;
static char *interleave_results_filename = def_irf;
static char *final_results_filename = def_frf;

static bool report_results_count = false;
static bool report_results_interleave = false;
static bool report_results_final = false;

static const char as_opts[] = "C:I:F:cif";
void parse_app_opt(char c, char *optarg)
{
    switch (c) {
        case 'C':
            count_results_filename = optarg;
        case 'c':
            report_results_count = true;
            break;
        case 'I':
            interleave_results_filename = optarg;
        case 'i':
            report_results_interleave = true;
            break;
        case 'F':
            final_results_filename = optarg;
        case 'f':
            report_results_final = true;
            break;
    }
}

extern int register_new;

int main(int argc, char **argv)
{
    AMI_err ae;

    parse_args(argc,argv,as_opts,parse_app_opt);

    if (verbose) {
        cout << "test_size = " << test_size << ".\n";
        cout << "test_mm_size = " << test_mm_size << ".\n";
        cout << "random_seed = " << random_seed << ".\n";
    } else {
        cout << test_size << ' ' << test_mm_size << ' ' << random_seed;
    }
    
    // Set the amount of main memory:
    MM_manager.resize_heap(test_mm_size);
    register_new = 1;
        
    AMI_STREAM<int> amis0((unsigned int)1, test_size);
    AMI_STREAM<int> amis1((unsigned int)1, test_size);
    AMI_STREAM<int> amis2((unsigned int)1, 2 * test_size);
    AMI_STREAM<int> amis3((unsigned int)1, 2 * test_size);

    // Streams for reporting values to ascii streams.
    
    ofstream *osc;
    ofstream *osi;
    ofstream *osf;
    cxx_ostream_scan<int> *rptc = NULL;
    cxx_ostream_scan<int> *rpti = NULL;
    cxx_ostream_scan<int> *rptf = NULL;
    
    if (report_results_count) {
        osc = new ofstream(count_results_filename);
        rptc = new cxx_ostream_scan<int>(osc);
    }
    
    if (report_results_interleave) {
        osi = new ofstream(interleave_results_filename);
        rpti = new cxx_ostream_scan<int>(osi);
    }
    
    if (report_results_final) {
        osf = new ofstream(final_results_filename);
        rptf = new cxx_ostream_scan<int>(osf);
    }
    
    // Write some ints.
    scan_count sc(test_size);

    ae = AMI_scan(&sc, &amis0);

    if (verbose) {
        cout << "Wrote the initial sequence of values.\n";
        cout << "Stopped (didn't write) with ii = "
             << sc.ii << ". operate() called " << sc.called << " times.\n";
        cout << "Stream length = " << amis0.stream_len() << '\n';
    }

    if (report_results_count) {
        ae = AMI_scan(&amis0, rptc);
    }
    
    // Square them.
    scan_square<int> ss;
        
    ae = AMI_scan(&amis0, &ss, &amis1);

    if (verbose) {
        cout << "Squared them; last squared was ii = "
             << ss.ii << ". operate() called " << ss.called << " times.\n";
        cout << "Stream length = " << amis1.stream_len() << '\n';
    }
    
    // Interleave the streams.
    merge_interleave<int> im;

    arity_t arity = 2;
        
    AMI_base_stream<int> *amirs[2];

    amirs[0] = &amis0;
    amirs[1] = &amis1;
    
    ae = AMI_single_merge((AMI_STREAM<int> **)amirs, arity, &amis2,
                          (AMI_merge_base<int> *)&im);

    if (verbose) {
        cout << "Interleaved them; operate() called " << im.called 
             << " times.\n";
        cout << "Stream length = " << amis2.stream_len() << '\n';
    }

    if (report_results_interleave) {
        ae = AMI_scan(&amis2, rpti);
    }

    // Divide the stream into two substreams, and interleave them.

    ae = amis2.new_substream(AMI_READ_STREAM, 0, test_size-1, &(amirs[0]));
    ae = amis2.new_substream(AMI_READ_STREAM, test_size, 2*test_size-1,
                             &(amirs[1]));

    if (verbose) {
        cout << "Created substreams; lengths = " <<
            ((AMI_STREAM<int> *)(amirs[0]))->stream_len() << " and " <<
                ((AMI_STREAM<int> *)(amirs[1]))->stream_len() << '\n';
    }

    // Get around the OS (HP_UX in particular) when using BTE_IMP_MMB
    // by seeking back to 0 in the substream, which will force the last
    // block written to be unmapped.
    ae = amis2.seek(0);
    
    ae = AMI_single_merge((AMI_STREAM<int> **)amirs, arity, &amis3,
                          (AMI_merge_base<int> *)&im);

    if (verbose) {
        cout << "Interleaved them; operate() called " << im.called 
             << " times.\n";        
        cout << "Stream length = " << amis3.stream_len() << '\n';
    }
    
    if (report_results_final) {
        ae = AMI_scan(&amis3, rptf);
    }

    delete amirs[0];
    delete amirs[1];
    
    return 0;
}



// Instantiate all the templates we have used.

#ifdef NO_IMPLICIT_TEMPLATES

// Instantiate templates for streams of objects.
TEMPLATE_INSTANTIATE_STREAMS(int)

// Instantiate templates for I/O using C++ streams.
TEMPLATE_INSTANTIATE_OSTREAM(int)

// Templated scan/merge management objects used by this program.
template class scan_square<int>;
template class merge_interleave<int>;

// Calls to AMI_scan using various object types.
template AMI_err AMI_scan(scan_count *, AMI_STREAM<int> *);
template AMI_err AMI_scan(AMI_STREAM<int> *, scan_square<int> *,
                          AMI_STREAM<int> *);

TEMPLATE_INSTANTIATE_MERGE(int)

#endif
