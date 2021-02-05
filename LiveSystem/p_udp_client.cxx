#include <stdio.h>
#include <thread>

#include <sodtp_jitter.h>
#include <udp_client.h>
#include <decode_video.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


using namespace std;



int main(int argc, char *argv[]) {
    JitterBuffer jbuffer;
    std::vector<thread *> thds;

    const char *host = argv[1];
    const char *port = argv[2];

    const char *path = NULL;
    if (argc >= 4) {
        path = argv[3];
    }

    SaveConfig scon;
    scon.parse("./config/save.conf");
    if (!path) {
        path = scon.path.c_str();
    }
    if (!scon.save) {
        path = NULL;
    }


    thread thread_dtp(udp_client, host, port, &jbuffer);


    bool found = false;
    do {
        // check for the semaphore in jitter buffer.
        // if we get semaphore, check the buffer lists
        // for NEW: create a decoding thread.
        // for CLOSE: release the corresponding SodtpJitter, note its thread
        // is already closed by itself.
        //

        printf("waiting ...\n");
        sem_wait(jbuffer.sem);
        printf("a new signal.\n");
        // for (auto ptr : jbuffer) {
        //     if ((ptr->state & SodtpJitter::STATE_INIT) == SodtpJitter::STATE_INIT) {
        //         thread *pthd = new thread(dtp_io, "127.0.0.1", "2333", &jbuffer);
        //         ptr->set_work_thread(pthd);
        //         break;
        //     }
        //     else if ((ptr->state & SodtpJitter::STATE_CLOSE) == SodtpJitter::STATE_CLOSE) {
        //         thread *pthd = ptr->get_work_thread();
        //         break;
        //     }
        // }

        // lock the jptrs.
        scoped_lock lock(jbuffer.mtx);

        found = false;
        auto it = jbuffer.jptrs.begin();
        printf("searching signal source, jitter queue number %lu\n", jbuffer.jptrs.size());
        while (it != jbuffer.jptrs.end()) {
            if (((*it)->state & SodtpJitter::STATE_INIT) == SodtpJitter::STATE_INIT &&
                (*it)->get_work_thread() == NULL) {

                thread *pthd = new thread(video_viewer3, *it, path);
                // thread *pthd = new thread(video_viewer, new SodtpJitter(0, NULL));
                // thread *pthd = new thread(dtp_client, "127.0.0.1", "2333", &jbuffer);
                (*it)->set_work_thread(pthd);
                thds.push_back(pthd);
                found = true;
                break;
            }
            // We DO NOT kill thread.
            // Thread should be stopped by itself due to the 'FIN' flag in block.
            else if (((*it)->state & SodtpJitter::STATE_CLOSE) == SodtpJitter::STATE_CLOSE) {
                // thread *pthd = (*it)->get_work_thread();
                it = jbuffer.jptrs.erase(it);
                printf("thread: state close\n");
                found = true;
                break;
            }
            else {
                ++it;
            }

        }

        if (found == false) {
            printf("Warning! Unable to find target.\n");
            printf("It could be the signal of connection closed. Exit now\n");
            break;
        }

    } while (jbuffer.jptrs.size() > 0);

    printf("clear network thread.\n");
    thread_dtp.join();

    printf("total decoding thread number %lu\n", thds.size());
    for (auto it : jbuffer.jptrs) {
        it->set_state(SodtpJitter::STATE_CLOSE);
    }
    for (auto it : thds) {
        it->join();
        printf("clear one decoding thread\n");
    }

    printf("main thread exit now\n");
    return 0;
}
