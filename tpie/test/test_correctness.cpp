// Copyright (C) 2003 Octavian Procopiuc
//
// File:    test_correctness.cpp
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// An extensive test suite for TPIE functionality.
//
// $Id: test_correctness.cpp,v 1.1 2003-04-24 23:48:07 tavi Exp $
//

using namespace std;

// For stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// For strlen()
#include <string.h>

#include <config.h>

#if (!defined(BTE_STREAM_IMP_UFS) && !defined(BTE_STREAM_IMP_MMAP) && !defined(BTE_STREAM_IMP_STDIO))
#  define BTE_STREAM_IMP_UFS
#endif

#define BTE_STREAM_UFS_BLOCK_FACTOR 32
#define BTE_STREAM_MMAP_BLOCK_FACTOR 32

// Use logs if requested.
#if TP_LOG_APPS
#  define TPL_LOGGING 1
#endif
#include <tpie_log.h>

// TPIE configuration: choose BTE, block size, etc.
//#include "app_config.h"
// TPIE core classes and functions.
#include <ami.h>
// The getopts() function for reading command-line arguments.
#include "getopts.h"
// The scan_random class for generating random ints.
#include "scan_universal.h"

// Number of spaces to indent messages written during a test.
#define INDENT 4

// Status of a test.
enum status_t {
  EMPTY = 0,
  PASS,
  FAIL,
  SKIP,
  NA
};

// Dummy type for streams.
template <int sz>
struct foo_t {
  char el[sz];
};

const foo_t<40> thefoo = { "  This space for rent. Cheap.          " };

struct options opts[] = {
  //  { 11, "stream-ufs", "Test streams (UFS BTE)", NULL, 0 },
  //  { 12, "stream-mmap", "Test streams (MMAP BTE)", NULL, 0 },
  //  { 13, "stream-stdio", "Test streams (STDIO BTE)", NULL, 0 },
  { 10, "stream", "Perform all stream tests", NULL, 0 },
  //{ 26, "scan-ascii-read", "Test ASCII reading (using cxx_istream_scan)", NULL, 0 },
  //{ 27, "scan-ascii-write", "Test ASCII writing (using cxx_ostream_scan)", NULL, 0 },
  { 20, "scan", "Perform all scanning tests", NULL, 0 },
  //  { 31, "sort-op-int-int", "Test standard sorting (using integers and operator \"<\")", NULL, 0 },
  //  { 32, "sort-op-100b-int", "Test AMI_sort (using 40-byte elements, integers as keys, and operator \"<\")", NULL, 0 },
  //  { 33, "sort-op-100b-100b", "Test AMI_sort (using 100-byte elements, and operator \"<\")", NULL, 0 },
  //  { 34, "sort-cl-int-int", "Test AMI_sort (using integers and comparison class)", NULL, 0 },
  //  { 35, "sort-op-100b-int", "Test AMI_sort (using 40-byte elements, integers as keys, and comparison class)", NULL, 0 },
  //  { 36, "sort-op-100b-100b", "Test AMI_sort (using 40-byte elements, and comparison class)", NULL, 0 },
  { 30, "sort", "Perform all AMI_sort tests", NULL, 0 },
  { 0,  NULL, NULL, NULL, 0 }
};

// The test functions.
int test_stream();
//int test_stream_ufs();
//int test_stream_mmap();
//int test_stream_stdio();
int test_scan();
// Print current configuration options.
void print_cfg();

int main(int argc, char **argv) {
  char *args;
  int idx;
  int fail = 0;

  // Initialize the log.
  LOG_SET_THRESHOLD(TP_LOG_APP_DEBUG);

  print_cfg();

  while ((idx = getopts(argc, argv, opts, &args)) != 0) {
    switch(idx) {
      //    case 11: fail += test_stream_ufs(); break;
      //    case 12: fail += test_stream_mmap(); break;
      //    case 13: fail += test_stream_stdio(); break;
    case 10: fail += test_stream(); break;
    case 20: fail += test_scan(); break;
    default: break;
    }

    free(args);
  }

  if (fail)
    fprintf(stdout, "One or more tests failed. See the log for more details.\n");
  else
    fprintf(stdout, "All test have completed successfully. See the log for more info.\n");

  return fail;
}


void print_msg(const char* msg, int indent = 0) {
  if (msg == NULL)
    return;
  int len = strlen(msg);
  if (indent < 0)
    for (int i=0; i<len; i++) fprintf(stdout, "\b");
  else
    for (int i=0; i<indent; i++) fprintf(stdout, " ");
  fprintf(stdout, msg);
  LOG_APP_DEBUG(">>-");
  LOG_APP_DEBUG(msg);
  LOG_APP_DEBUG("\n");
  if (indent >= 0) {
    int current_pos = indent + len;
    for (int i=current_pos; i<74; i++) fprintf(stdout, " ");
  }
  fflush(stdout);
}

void print_status(status_t status) {
  switch (status) {
  case EMPTY: 
    fprintf(stdout, "\n"); 
    LOG_APP_DEBUG(">>-\n");
    break;
  case SKIP:  
    fprintf(stdout, " [SKIP]\n"); 
    LOG_APP_DEBUG(">>-[SKIP]\n");
    break;
  case PASS:  
    // This prints in green, when possible.
    fprintf(stdout, " \033[1;32m[PASS]\033[0m\n"); 
    LOG_APP_DEBUG(">>-[PASS]\n");
    break;
  case FAIL:  
    // This prints in red, when possible.
    fprintf(stdout, " \033[1;31m[FAIL]\033[0m\n"); 
    LOG_APP_DEBUG(">>-[FAIL]\n");
    break;
  case NA:    
    fprintf(stdout, " [N/Av]\n"); 
    LOG_APP_DEBUG(">>-[N/Av]\n");
    break;
  }
}

void print_cfg() {
  fprintf(stdout, "TPIE Configuration\n");
  fprintf(stdout, "    Stream BTE: ");
#if defined(BTE_STREAM_IMP_UFS)
  fprintf(stdout, "UFS (%d)", BTE_STREAM_UFS_BLOCK_FACTOR);
#elif defined(BTE_STREAM_IMP_MMAP)
  fprintf(stdout, "MMAP (%d)", BTE_STREAM_MMAP_BLOCK_FACTOR);
#elif defined(BTE_STREAM_IMP_STDIO)
  fprintf(stdout, "STDIO");
#elif defined(BTE_STREAM_IMP_USER_DEFINED)
  fprintf(stdout, "USER_DEFINED");
#else
  fprintf(stdout, "UNSPECIFIED");
#endif
  fprintf(stdout, "\n");
  fprintf(stdout, "    TPIE Memory limit: %d KB\n", MM_manager.memory_limit()/1024);
#if TP_LOG_APPS
  fprintf(stdout, "    Logging in file %s\n", theLogName());
#else
  fprintf(stdout, "    Logging is OFF\n");
#endif
  fprintf(stdout, "\n");
}

int test_stream() {
  static bool been_here = false;
  status_t status = EMPTY;
  AMI_STREAM<foo_t<40> >* s;
  int failed = 0;
  AMI_err err;
  char fn[] = "/var/tmp/tpie00.stream";
  char *pfn = NULL; // Pointer to a file name.
  foo_t<40> afoo = thefoo;
  foo_t<40> *pafoo;
  int i;
  struct stat buf;

  print_msg("Testing AMI_STREAM creation and destruction", 0);
  if (been_here) {
    print_status(SKIP);
    return 0;
  }

  been_here = true;
  print_status(EMPTY);


  ///////////////////////////////////////////////////
  //////// Part 1: temporary stream.       //////////
  ///////////////////////////////////////////////////

  print_msg("Creating temporary stream (calling op. new)", INDENT);
  s = new AMI_STREAM<foo_t<40> >;
  status = (s != NULL && s->is_valid() ? PASS: FAIL);
  print_status(status); if (status == FAIL) failed++;

  if (status != FAIL) {

    print_msg("Checking stream_len() (should return 0)", INDENT);
    status = (s->stream_len() == 0 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;
  
    print_msg("Inquiring file name: ", INDENT);
    err = s->name(&pfn);
    status = (err != AMI_ERROR_NO_ERROR || pfn == NULL || strlen(pfn) == 0 ? FAIL: PASS);
    if (pfn != NULL) print_msg(pfn, -1);
    print_status(status); if (status == FAIL) failed++;

    print_msg("Checking persist() (should return PERSIST_DELETE)", INDENT);
    status = (s->persist() == PERSIST_DELETE ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;    
    
    print_msg("Checking write_item() (writing 1M items)", INDENT);
    err = AMI_ERROR_NO_ERROR;
    for (i=0; i<1000000; i++) {
      afoo.el[0] = i % 128;
      afoo.el[38] = (i+5) % 128;
      if ((err = s->write_item(afoo)) != AMI_ERROR_NO_ERROR)
	break;
    }
    status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == 1000000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking seek() (seeking to illegal position 1000001)", INDENT);
    err = s->seek(1000001);
    status = (err == AMI_ERROR_NO_ERROR ? FAIL: PASS);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking seek() (seeking to position 50000)", INDENT);
    err = s->seek(50000);
    status = (err == AMI_ERROR_NO_ERROR ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking read_item() (reading 10k items from current pos)", INDENT);
    for (i=50000; i<60000; i++) {
      err = s->read_item(&pafoo);
      if (err != AMI_ERROR_NO_ERROR || pafoo->el[0] != (i%128) || pafoo->el[38] != ((i+5)%128))
	break;
    }
    status = (err == AMI_ERROR_NO_ERROR && i == 60000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking tell() (should return 60000)", INDENT);
    status = (s->tell() == 60000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking truncate() (to 50000)", INDENT);
    err = s->truncate(50000);
    status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == 50000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking tell() again (should return 50000)", INDENT);
    status = (s->tell() == 50000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

  }
  
  print_msg("Destroying temp stream (file should be removed) (calling op. delete)", INDENT);
  delete s;
  status = (stat(pfn, &buf) == -1 && errno == ENOENT ? PASS: FAIL);
  delete pfn;
  print_status(status); if (status == FAIL) failed++;


  print_status(EMPTY); // New line.

  ///////////////////////////////////////////////////
  //////// Part 2: named stream.           //////////
  ///////////////////////////////////////////////////

  print_msg("Creating named writable stream (calling op. new)", INDENT);
  // Make sure there's no old file lingering around.
  unlink(fn);
  s = new AMI_STREAM<foo_t<40> >(fn);
  status = (s != NULL && s->is_valid() ? PASS: FAIL);
  print_status(status); if (status == FAIL) failed++;

  if (status != FAIL) {

    print_msg("Inquiring file name; should be called", INDENT);
    print_msg(fn, -1);
    err = s->name(&pfn);
    status = (err != AMI_ERROR_NO_ERROR || pfn == NULL || strcmp(pfn, fn) != 0 ? FAIL: PASS);
    if (pfn != NULL) print_msg(pfn, -1);
    print_status(status); if (status == FAIL) failed++;

    print_msg("Checking persist() (should return PERSIST_PERSISTENT)", INDENT);
    status = (s->persist() == PERSIST_PERSISTENT ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;    
 
    print_msg("Checking write_item() (writing 1M items)", INDENT);
    err = AMI_ERROR_NO_ERROR;
    for (i=0; i<1000000; i++) {
      afoo.el[0] = i % 128;
      afoo.el[38] = (i+5) % 128;
      if ((err = s->write_item(afoo)) != AMI_ERROR_NO_ERROR) break;
    }
    status = (err == AMI_ERROR_NO_ERROR && s->stream_len() == 1000000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++; 

    print_msg("Checking illegal read_item() at current pos", INDENT);
    err = s->read_item(&pafoo);
    status = (err == AMI_ERROR_END_OF_STREAM ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;

  }

  print_msg("Closing named stream (file should NOT be removed) (calling op. delete)", INDENT);
  delete s;
  status = (stat(pfn, &buf) == 0 ? PASS: FAIL);
  delete pfn;
  print_status(status); if (status == FAIL) failed++;

  print_msg("Reopening named stream read-only (calling op. new)", INDENT);
  s = new AMI_STREAM<foo_t<40> >(fn, AMI_READ_STREAM);
  status = (s != NULL && s->is_valid() ? PASS: FAIL);
  print_status(status); if (status == FAIL) failed++;  
  
  if (status != FAIL) {

    print_msg("Checking stream_len() (should return 1M)", INDENT);
    status = (s->stream_len() == 1000000 ? PASS: FAIL);
    print_status(status); if (status == FAIL) failed++;

    print_msg("Checking illegal write_item() in read-only stream", INDENT);
    err = s->write_item(afoo);
    status = (err == AMI_ERROR_NO_ERROR || s->stream_len() != 1000000 ? FAIL: PASS);
    print_status(status); if (status == FAIL) failed++;
    
  }

  print_msg("Closing named stream (file should NOT be removed) (calling op. delete)", INDENT);
  delete s;
  status = (stat(fn, &buf) == 0 ? PASS: FAIL);
  print_status(status); if (status == FAIL) failed++;  


  print_msg("Reopening named stream for reading and writing (calling op. new)", INDENT);
  s = new AMI_STREAM<foo_t<40> >(fn, AMI_READ_WRITE_STREAM);
  status = (s != NULL && s->is_valid() ? PASS: FAIL);
  print_status(status); if (status == FAIL) failed++;  
  
  if (status != FAIL) {

    print_msg("Calling persist(PERSIST_DELETE) to set persistency", INDENT);
    s->persist(PERSIST_DELETE);
    print_status(PASS);

  }

  print_msg("Destroying named stream (file should be removed) (calling op. delete)", INDENT);
  delete s;
  status = (stat(fn, &buf) == -1 && errno == ENOENT ? PASS: FAIL);
  print_status(status); if (status == FAIL) failed++;
  
  print_status(EMPTY);

  return (failed ? 1: 0);
}


int test_scan() {
  static bool been_here = false;
  status_t status = EMPTY;
  int failed = 0;
  AMI_STREAM<foo_t<40> > *s;
  AMI_STREAM<int> *ps[9];
  AMI_err err;
  int i;
  scan_universal so(10000000, 43); // scan object.


  print_msg("Testing AMI_scan", 0);
  if (been_here) {
    print_status(SKIP);
    return 0;
  }
  been_here = true;
  print_status(EMPTY); // New line.


  print_msg("Initializing temporary streams.", INDENT);
  for (i = 0; i < 9; i++) {
    ps[i] = new AMI_STREAM<int>;
    if (!ps[i]->is_valid()) {
      status = FAIL;
      break;
    }
  }
  print_status(status);
  if (status == FAIL) { failed++; status = SKIP; }

  
  print_msg("Running AMI_scan with 0 in and 1 out (10m random integers)", INDENT);
  if (status != SKIP) {
    err = AMI_scan(&so, ps[0]);
    status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan with 1 in and 0 out (counting even and odd)", INDENT);
  if (status != SKIP) {
    err = AMI_scan(ps[0], &so);
    status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 && so.even() + so.odd() == 10000000 ? PASS: FAIL);
    LOG_APP_DEBUG_ID("Even integers:");
    LOG_APP_DEBUG_ID(so.even());
    LOG_APP_DEBUG_ID("Odd integers:");
    LOG_APP_DEBUG_ID(so.odd());
  }
  print_status(status); if (status == FAIL) failed++;
  
  print_msg("Running AMI_scan with 1 in and 1 out (halving each of 10m integers)", INDENT);
  if (status != SKIP) {
    err = AMI_scan(ps[0], &so, ps[1]);
    status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 && ps[1]->stream_len() == 10000000 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;

  
  print_msg("Checking streams integrity and current pos (should be 10000000)", INDENT);
  if (status != SKIP) {
    status = (ps[0]->is_valid() && ps[0]->stream_len() == 10000000 && ps[0]->tell() == 10000000  && ps[1]->tell() == 10000000 ? PASS: FAIL);
    LOG_APP_DEBUG_ID("Current position in input stream:");
    LOG_APP_DEBUG_ID(ps[0]->tell());
    LOG_APP_DEBUG_ID("Current position in output stream:");
    LOG_APP_DEBUG_ID(ps[1]->tell());
  }
  print_status(status); if (status == FAIL) failed++;
  

  print_msg("Running AMI_scan with 2 in and 1 out (min of 10m pairs of integers)", INDENT);
  if (status != SKIP) {
    err = AMI_scan(ps[0], ps[1], &so, ps[2]);
    status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 10000000 && ps[1]->stream_len() == 10000000 && ps[2]->stream_len() == 10000000 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Same as above, with non-equal-size inputs: 9m and 10m  ", INDENT);
  if (status != SKIP) {
    ps[0]->truncate(9000000);
    err = AMI_scan(ps[0], ps[1], &so, ps[3]);
    status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 9000000 && ps[1]->stream_len() == 10000000 && ps[3]->stream_len() == 10000000 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;

  print_msg("Running AMI_scan with 2 in and 2 out (off-synch: even and odd)", INDENT);
  if (status != SKIP) {
    err = AMI_scan(ps[0], ps[1], &so, ps[4], ps[5]);
    status = (err == AMI_ERROR_NO_ERROR && ps[0]->stream_len() == 9000000 && ps[1]->stream_len() == 10000000 && ps[4]->stream_len() + ps[5]->stream_len() == 19000000 ? PASS: FAIL);
    LOG_APP_DEBUG_ID("Length of \"even\" stream: ");
    LOG_APP_DEBUG_ID(ps[4]->stream_len());
    LOG_APP_DEBUG_ID("Length of \"odd\" stream: ");
    LOG_APP_DEBUG_ID(ps[5]->stream_len());
    LOG_APP_DEBUG_ID("The sum should be 19000000.");
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan with 1 in and 0 out to verify \"even\" stream", INDENT);
  if (status != SKIP) {
    err = AMI_scan(ps[4], &so);
    status = (err == AMI_ERROR_NO_ERROR && so.even() == ps[4]->stream_len() && so.odd() == 0 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan with 1 in and 0 out to verify \"odd\" stream", INDENT);
  if (status != SKIP) {
    err = AMI_scan(ps[5], &so);
    status = (err == AMI_ERROR_NO_ERROR && so.odd() == ps[5]->stream_len() && so.even() == 0 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan with 4 in and 3 out (avg, min, max)", INDENT);
  if (status != SKIP) {
    ps[3]->truncate(2000000);
    err = AMI_scan(ps[0], ps[1], ps[2], ps[3], &so, ps[6], ps[7], ps[8]);
    status = (err == AMI_ERROR_NO_ERROR && ps[6]->stream_len() == 10000000 && ps[7]->stream_len() == 10000000 && ps[8]->stream_len() == 10000000 ? PASS: FAIL);
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan illegally with same stream in and out", INDENT);
  LOG_APP_DEBUG_ID("Length of stream before scan:");
  LOG_APP_DEBUG_ID(ps[1]->stream_len());
  if (status != SKIP) {
    err = AMI_scan(ps[1], &so, ps[1]);
    status = (err == AMI_ERROR_NO_ERROR ? FAIL: PASS);
  }
  LOG_APP_DEBUG_ID("Length of stream after scan:");
  LOG_APP_DEBUG_ID(ps[1]->stream_len());  
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan illegally with non-valid in-stream", INDENT);
  if (status != SKIP) {
    AMI_STREAM<int> *psn = new AMI_STREAM<int>("/glkdjldas");
    err = AMI_scan(psn, &so);
    status = (err == AMI_ERROR_NO_ERROR && !psn->is_valid() ? FAIL: PASS);
    delete psn;
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan illegally with non-valid out-stream", INDENT);
  if (status != SKIP) {
    AMI_STREAM<int> *psn = new AMI_STREAM<int>("/glkdjldas");
    err = AMI_scan(ps[2], &so, psn);
    status = (err == AMI_ERROR_NO_ERROR && !psn->is_valid() ? FAIL: PASS);
    delete psn;
  }
  print_status(status); if (status == FAIL) failed++;


  print_msg("Running AMI_scan illegally with read-only out-stream", INDENT);
  if (status != SKIP) {
    AMI_STREAM<int> *psn = new AMI_STREAM<int>;
    psn->persist(PERSIST_PERSISTENT);
    char *fn;
    psn->name(&fn);
    delete psn;
    psn = new AMI_STREAM<int>(fn, AMI_READ_STREAM);
    if (!psn->is_valid())
      status = FAIL;
    else {
      err = AMI_scan(ps[3], &so, psn);
      status = (err == AMI_ERROR_NO_ERROR ? FAIL: PASS);
    }
    delete fn;
    psn->persist(PERSIST_DELETE);
    delete psn;
  }
  print_status(status); if (status == FAIL) failed++;


  for (i = 0; i < 9; i++)
    delete ps[i];

  return (failed ? 1: 0);
}
