/*
   Original author of the starter code
   Tanzir Ahmed
   Department of Computer Science & Engineering
   Texas A&M University
   Date: 2/8/20


   Please include your Name, UIN, and the date below
   Name: Suhas Guddeti
   UIN: 235004809
   Date: 9/28/2025
*/
#include "FIFORequestChannel.h"
#include "common.h"
#include <memory>
#include <sys/wait.h>

using namespace std;

static void make_sure_we_get_directory() {
    struct stat st{};
    if (stat("received", &st) == -1) {
        mkdir("received", 0777); // Linux permission thing, read later
    }
}

static pid_t server_spinup(int buffercap) {
    pid_t pid = fork();
    if (pid < 0) {
        EXITONERROR("fork");
    }
    if (pid == 0) {
        vector<char *> args;
        args.push_back(const_cast<char *>("./server")); // i don't know why this can't be string
        string mval;
        if (buffercap != MAX_MESSAGE) {
            args.push_back(const_cast<char *>("-m"));
            mval = to_string(buffercap); // this segfaults in test, check out later
            args.push_back(const_cast<char *>(mval.c_str()));
        }
        args.push_back(nullptr);
        execvp(args[0], args.data());
        EXITONERROR("execvp ./server");
    }
    return pid;
}

static unique_ptr<FIFORequestChannel> make_new_channel(FIFORequestChannel &control) {
    MESSAGE_TYPE m = NEWCHANNEL_MSG;
    control.cwrite(&m, sizeof(m)); // just make a NEWCHANNEL_MSG and hope server likes it
    char namebuf[256];
    int n = control.cread(namebuf, sizeof(namebuf));
    if (n <= 0) {
        EXITONERROR("cread new channel name");
    }
    return make_unique<FIFORequestChannel>(string(namebuf), FIFORequestChannel::CLIENT_SIDE);
}

static double get(FIFORequestChannel &chan, int p, double t, int e) {
    datamsg req(p, t, e);
    chan.cwrite(&req, sizeof(req));
    double resp = 0.0;
    chan.cread(&resp, sizeof(resp));
    return resp;
}

static void get_big(FIFORequestChannel &chan, int p) {
    make_sure_we_get_directory();
    ofstream ofs("received/x1.csv"); // remove hardcoding later
    if (!ofs)
        EXITONERROR("open received/x1.csv");

    for (int i = 0; i < 1000; ++i) {
        double tt = i * 0.004;
        double ecg1 = get(chan, p, tt, 1);
        double ecg2 = get(chan, p, tt, 2);
        ofs << tt << "," << ecg1 << "," << ecg2 << "\n"; // newline might be an issue, fix later
    }
}

static __int64_t get_file_size(FIFORequestChannel &chan, const string &fname) {
    filemsg fm(0, 0);
    int len = sizeof(filemsg) + (int)fname.size() + 1;
    vector<char> buf(len);
    memcpy(buf.data(), &fm, sizeof(filemsg));
    strcpy(buf.data() + sizeof(filemsg), fname.c_str());
    chan.cwrite(buf.data(), len);
    __int64_t fsz = 0;
    chan.cread(&fsz, sizeof(fsz));
    return fsz;
}

static void download(FIFORequestChannel &chan, const string &fname, int buffercap) {
    make_sure_we_get_directory();

    string justname = fname;
    const string prefix = "BIMDC/";
    if (justname.rfind(prefix, 0) == 0)
        justname = justname.substr(prefix.size());

    __int64_t fsz = get_file_size(chan, justname);

    string outpath = "received/" + justname;
    FILE *out = fopen(outpath.c_str(), "wb"); // old school C file handling bc why not
    if (!out)
        EXITONERROR("open output file");

    vector<char> reqbuf(sizeof(filemsg) + (int)justname.size() + 1);
    vector<char> rbuf(buffercap);

    __int64_t offset = 0;
    while (offset < fsz) {
        int chunk = (int)min<__int64_t>(buffercap, fsz - offset);

        filemsg fm(offset, chunk);
        int len = sizeof(filemsg) + (int)justname.size() + 1;
        if ((int)reqbuf.size() != len)
            reqbuf.resize(len);
        memcpy(reqbuf.data(), &fm, sizeof(filemsg));
        strcpy(reqbuf.data() + sizeof(filemsg), justname.c_str());

        chan.cwrite(reqbuf.data(), len);
        int nread = chan.cread(rbuf.data(), chunk);
        if (nread < 0)
            EXITONERROR("file chunk read");
        size_t wrote = fwrite(rbuf.data(), 1, nread, out);
        if ((int)wrote != nread)
            EXITONERROR("fwrite");
        offset += nread; // just keep shoving bytes forward
    }

    fclose(out);
}

int main(int argc, char *argv[]) {
    int opt;
    int p = 1;
    double t = 0.0;
    int e = 1;

    string filename = "";
    bool did_we_use_a_new_channel = false;
    int buffercapacity = MAX_MESSAGE;
    while ((opt = getopt(argc, argv, "p:t:e:f:cm:")) != -1) {
        switch (opt) {
        case 'p':
            p = atoi(optarg);
            break;
        case 't':
            t = atof(optarg);
            break;
        case 'e':
            e = atoi(optarg);
            break;
        case 'f':
            filename = optarg;
            break;
        case 'c':
            did_we_use_a_new_channel = true;
            break;
        case 'm':
            buffercapacity = atoi(optarg);
            break;
        }
    }

    pid_t srv = server_spinup(buffercapacity);
    FIFORequestChannel control("control", FIFORequestChannel::CLIENT_SIDE);
    unique_ptr<FIFORequestChannel> newchan;
    FIFORequestChannel *active = &control;
    if (did_we_use_a_new_channel) {
        newchan = make_new_channel(control);
        active = newchan.get();
    }

    bool bruh = false;
    bool lol = false;
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "-t")
            bruh = true;
        if (string(argv[i]) == "-e")
            lol = true;
    }

    if (!filename.empty()) {
        download(*active, filename, buffercapacity);
    } else if (bruh && lol) {
        double val = get(*active, p, t, e);
        cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << val << endl;
    } else if (optind > 1 || p != 1) {
        get_big(*active, p);
    }

    if (newchan) {
        MESSAGE_TYPE m = QUIT_MSG;
        newchan->cwrite(&m, sizeof(m));
        newchan.reset();
    }

    MESSAGE_TYPE m = QUIT_MSG;
    control.cwrite(&m, sizeof(m));

    int status = 0;
    waitpid(srv, &status, 0); // sit here and just wait for server to die
    return 0;
}