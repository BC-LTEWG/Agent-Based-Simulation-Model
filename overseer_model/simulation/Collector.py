import logging
logger = logging.getLogger(__name__)
import queue, os, json
from dataclasses import dataclass
import subprocess, threading

@dataclass
class StreamItem:
    stream: str
    kind: str
    payload: object

class Collector:
    """ 
    Container class that continually unloads the standard output/standard error of 
    the simulation into a queue, where it will be safe until ready for processing
    """
    def __init__(self, bin_path: str, args: list = ["-j"]):
        self.output_queue = queue.Queue(maxsize= 10000)
        self.bin_path = bin_path
        self.args = args

    def get_next(self):
        """ Tiny wrapper for dequeueing """
        try:
            return self.output_queue.get(timeout= 0.05)
        except queue.Empty:
            return StreamItem("meta", "wait", None)

    def start_sim_and_begin_collection(self):
        """ 
        Starts the subprocess, creates threads for standard output and standard error, 
         and starts collecting.
        """
        if os.name == "nt":
            self.proc = subprocess.Popen(
                ["cmd", "/c", self.bin_path, *self.args],
                stdout= subprocess.PIPE,
                stderr= subprocess.PIPE,
                creationflags=subprocess.CREATE_NO_WINDOW,
                text= True,
                bufsize= 1,
            )
        else:
            self.proc = subprocess.Popen(
                [self.bin_path, *self.args],
                stdout= subprocess.PIPE,
                stderr= subprocess.PIPE,
                text= True,
                bufsize= 1,
                start_new_session= True
            )
        assert self.proc.stdout is not None
        
        self.out_thread = threading.Thread(
            target= self.collect,
            args= (self.proc.stdout, "stdout"),
            daemon= True,
        )
        self.err_thread = threading.Thread(
            target= self.collect,
            args= (self.proc.stderr, "stderr"),
            daemon= True,
        )

        self.out_thread.start()
        self.err_thread.start()

        return self.proc

    def collect(self, pipe, stream_name):
        """ Collect standard output/error, unload it into the queue """
        try:
            for line in pipe:
                line = line.strip()
                if not line:
                    continue

                if stream_name == "stdout":
                    try:
                        self.output_queue.put(StreamItem("stdout", "json", json.loads(line)))
                    except json.JSONDecodeError:
                        self.output_queue.put(StreamItem("stdout", "text", line))
                else:
                    self.output_queue.put(StreamItem("stderr", "text", line))

        except Exception as e:
            self.output_queue.put(StreamItem(stream_name, "error", e))
        finally:
            try:
                pipe.close()
            except Exception:
                pass
            self.output_queue.put(StreamItem(stream_name, "eof", None))

    def stop(self, timeout: float = 1.0):
        proc = getattr(self, "proc", None)
        if proc is None:
            return True

        if proc.poll() is not None:
            return True

        try:
            if os.name == "nt":
                proc.terminate()
            else:
                import signal
                os.killpg(proc.pid, signal.SIGTERM)

            try:
                proc.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                if os.name == "nt":
                    proc.kill()
                else:
                    os.killpg(proc.pid, signal.SIGKILL)
                proc.wait(timeout=timeout)
        finally:
            for pipe_name in ("stdout", "stderr"):
                pipe = getattr(proc, pipe_name, None)
                if pipe is not None:
                    try:
                        pipe.close()
                    except Exception:
                        pass

            for thread_name in ("out_thread", "err_thread"):
                thread = getattr(self, thread_name, None)
                if thread is not None:
                    try:
                        thread.join(timeout=0.25)
                    except RuntimeError:
                        pass

        return proc.poll() is not None
