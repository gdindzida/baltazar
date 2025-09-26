import yaml
import sys
import argparse
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
from typing import Tuple, Dict, Any
from numpy.typing import NDArray
from enum import IntEnum

class JobColumns(IntEnum):
    TASK_ID = 0
    JOB_ID = 1
    THREAD_ID = 2
    SCHED_DURATION = 3
    RUN_DURATION = 4
    WAIT_DURATION = 5

class WaveColumns(IntEnum):
    WAVE_NUM = 0
    DURATION = 1

class TaskStatsColumns(IntEnum):
    SCHED_DURATION = 0
    RUN_DURATION = 1
    WAIT_DURATION = 2

class ThreadStatsColumns(IntEnum):
    RUN_DURATION = 0


def compute_job_stats(arr: NDArray[np.float64]) -> tuple[Dict[str, Any], Dict[int, NDArray[np.uint64]], Dict[int, NDArray[np.uint64]]]:
    """Compute basic statistics for a jobs array."""
    if arr.size == 0:
        return {}, {}, {}

    task_counts : Dict[int, int]= {}
    thread_counts : Dict[int, int] = {}

    for row in arr:
        task_id = row[JobColumns.TASK_ID]
        thread_id = row[JobColumns.THREAD_ID]

        if task_id in task_counts:
            task_counts[task_id] += 1
        else:
            task_counts[task_id] = 1

        if thread_id in thread_counts:
            thread_counts[thread_id] += 1
        else:
            thread_counts[thread_id] = 1

    task_stats : Dict[int, NDArray[np.uint64]] = {}
    thread_stats : Dict[int, NDArray[np.uint64]] = {}

    task_sizes = {}
    thread_sizes = {}

    for row in arr:
        task_id = row[JobColumns.TASK_ID]
        thread_id = row[JobColumns.THREAD_ID]

        if task_id in task_stats:
            task_stats[task_id][task_sizes[task_id]][TaskStatsColumns.SCHED_DURATION] = row[JobColumns.SCHED_DURATION] 
            task_stats[task_id][task_sizes[task_id]][TaskStatsColumns.RUN_DURATION] = row[JobColumns.RUN_DURATION] 
            task_stats[task_id][task_sizes[task_id]][TaskStatsColumns.WAIT_DURATION] = row[JobColumns.WAIT_DURATION] 
            task_sizes[task_id] += 1
        else:
            task_stats[task_id] = np.zeros((task_counts[task_id], 3), dtype=np.uint64)
            task_stats[task_id][0][TaskStatsColumns.SCHED_DURATION] = row[JobColumns.SCHED_DURATION]
            task_stats[task_id][0][TaskStatsColumns.RUN_DURATION] = row[JobColumns.RUN_DURATION]
            task_stats[task_id][0][TaskStatsColumns.WAIT_DURATION] = row[JobColumns.WAIT_DURATION]
            task_sizes[task_id] = 1
            

        if thread_id in thread_stats:
            thread_stats[thread_id][thread_sizes[thread_id]][ThreadStatsColumns.RUN_DURATION] = row[JobColumns.RUN_DURATION]
            thread_sizes[thread_id] += 1
        else:
            thread_stats[thread_id] = np.zeros((thread_counts[thread_id], 1), dtype=np.uint64)
            thread_stats[thread_id][0][ThreadStatsColumns.RUN_DURATION] = row[JobColumns.RUN_DURATION]
            thread_sizes[thread_id] = 1

    stats : Dict[str, Any] = {}

    per_task_stats : Dict[int | str, Any]= {}
    for task_id in task_stats:
        task_entry : Dict[str, float | int] = {}
        task_entry["sched_time_mean"] = float(np.mean(task_stats[task_id][:, TaskStatsColumns.SCHED_DURATION]))
        task_entry["run_time_mean"] = float(np.mean(task_stats[task_id][:, TaskStatsColumns.RUN_DURATION]))
        task_entry["wait_time_mean"] = float(np.mean(task_stats[task_id][:, TaskStatsColumns.WAIT_DURATION]))
        task_entry["sched_time_median"] = float(np.median(task_stats[task_id][:, TaskStatsColumns.SCHED_DURATION]))
        task_entry["run_time_median"] = float(np.median(task_stats[task_id][:, TaskStatsColumns.RUN_DURATION]))
        task_entry["wait_time_median"] = float(np.median(task_stats[task_id][:, TaskStatsColumns.WAIT_DURATION]))
        task_entry["sched_time_std"] = float(np.std(task_stats[task_id][:, TaskStatsColumns.SCHED_DURATION]))
        task_entry["run_time_std"] = float(np.std(task_stats[task_id][:, TaskStatsColumns.RUN_DURATION]))
        task_entry["wait_time_std"] = float(np.std(task_stats[task_id][:, TaskStatsColumns.WAIT_DURATION]))
        task_entry["sched_time_min"] = int(np.min(task_stats[task_id][:, TaskStatsColumns.SCHED_DURATION]))
        task_entry["run_time_min"] = int(np.min(task_stats[task_id][:, TaskStatsColumns.RUN_DURATION]))
        task_entry["wait_time_min"] = int(np.min(task_stats[task_id][:, TaskStatsColumns.WAIT_DURATION]))
        task_entry["sched_time_max"] = int(np.max(task_stats[task_id][:, TaskStatsColumns.SCHED_DURATION]))
        task_entry["run_time_max"] = int(np.max(task_stats[task_id][:, TaskStatsColumns.RUN_DURATION]))
        task_entry["wait_time_max"] = int(np.max(task_stats[task_id][:, TaskStatsColumns.WAIT_DURATION]))
        per_task_stats[int(task_id)] = task_entry

    stats["task_stats"] = per_task_stats

    per_thread_stats : Dict[int, Any]= {}
    for thread_id in thread_stats:
        thread_entry : Dict[str, float | int] = {}
        thread_entry["run_time_mean"] = float(np.mean(thread_stats[thread_id][:, ThreadStatsColumns.RUN_DURATION]))
        thread_entry["run_time_median"] = float(np.median(thread_stats[thread_id][:, ThreadStatsColumns.RUN_DURATION]))
        thread_entry["run_time_std"] = float(np.std(thread_stats[thread_id][:, ThreadStatsColumns.RUN_DURATION]))
        thread_entry["run_time_min"] = int(np.min(thread_stats[thread_id][:, ThreadStatsColumns.RUN_DURATION]))
        thread_entry["run_time_max"] = int(np.max(thread_stats[thread_id][:, ThreadStatsColumns.RUN_DURATION]))
        per_thread_stats[int(thread_id)] = thread_entry

    stats["thread_stats"] = per_thread_stats

    sched_stats : Dict[str, float | int] = {}
    sched_stats["mean"] = float(np.mean(arr[:, JobColumns.SCHED_DURATION]))
    sched_stats["median"] = float(np.median(arr[:, JobColumns.SCHED_DURATION]))
    sched_stats["std"] = float(np.std(arr[:, JobColumns.SCHED_DURATION]))
    sched_stats["min"] = int(np.min(arr[:, JobColumns.SCHED_DURATION]))
    sched_stats["max"] = int(np.max(arr[:, JobColumns.SCHED_DURATION]))

    stats["sched_stats"] = sched_stats

    run_stats : Dict[str, float | int] = {}
    run_stats["mean"] = float(np.mean(arr[:, JobColumns.RUN_DURATION]))
    run_stats["median"] = float(np.median(arr[:, JobColumns.RUN_DURATION]))
    run_stats["std"] = float(np.std(arr[:, JobColumns.RUN_DURATION]))
    run_stats["min"] = int(np.min(arr[:, JobColumns.RUN_DURATION]))
    run_stats["max"] = int(np.max(arr[:, JobColumns.RUN_DURATION]))

    stats["run_stats"] = run_stats

    wait_stats : Dict[str, float | int] = {}
    wait_stats["mean"] = float(np.mean(arr[:, JobColumns.WAIT_DURATION]))
    wait_stats["median"] = float(np.median(arr[:, JobColumns.WAIT_DURATION]))
    wait_stats["std"] = float(np.std(arr[:, JobColumns.WAIT_DURATION]))
    wait_stats["min"] = int(np.min(arr[:, JobColumns.WAIT_DURATION]))
    wait_stats["max"] = int(np.max(arr[:, JobColumns.WAIT_DURATION]))

    stats["wait_stats"] = wait_stats

    return stats, task_stats, thread_stats

def compute_wave_stats(arr: NDArray[np.float64]) -> Dict[str, float | int]:
    """Compute basic statistics for a waves array."""
    if arr.size == 0:
        return {}

    wave_stats : Dict[str, float | int] = {}
    wave_stats["mean"] = float(np.mean(arr[:, WaveColumns.DURATION]))
    wave_stats["median"] = float(np.median(arr[:, WaveColumns.DURATION]))
    wave_stats["std"] = float(np.std(arr[:, WaveColumns.DURATION]))
    wave_stats["min"] = int(np.min(arr[:, WaveColumns.DURATION]))
    wave_stats["max"] = int(np.max(arr[:, WaveColumns.DURATION]))

    return wave_stats


def parse_file(filename: str) -> Tuple[NDArray[np.float64], NDArray[np.float64]]:
    """Read and parse the input file into W (wave) and J (job) arrays."""
    w_data: list[list[float]] = []
    j_data: list[list[float]] = []

    with open(filename, "r") as f:
        for line in f:
            parts = line.strip().split(",")
            if not parts:
                continue

            if parts[0] == "W":
                values = [float(x) for x in parts[1:] if x.strip() != ""]
                w_data.append(values)
            elif parts[0] == "J":
                values = [float(x) for x in parts[1:] if x.strip() != ""]
                j_data.append(values)

    w_array: NDArray[np.float64] = np.array(w_data) if w_data else np.empty((0,))
    j_array: NDArray[np.float64] = np.array(j_data) if j_data else np.empty((0,))
    return w_array, j_array


def save_stats_to_yaml(stats: Dict[str, Any], filename: str) -> None:
    """Save stats dictionary to a YAML file."""
    with open(filename, "w") as f:
        yaml.dump(stats, f)


def load_node_map(path: str) -> dict[int, str]:
    mapping: dict[int, str] = {}
    for line in Path(path).read_text().splitlines():
        if not line.strip():
            continue
        idx_str, name = line.split(":", 1)
        mapping[int(idx_str.strip())] = name.strip()
    return mapping

def create_histograms(arr: NDArray[np.uint64 | np.float64], prefix: str):
    num_cols = arr.shape[1]

    for i in range(num_cols):
        plt.figure(figsize=(6, 4) )# type: ignore
        plt.hist(arr[:, i], bins=20, edgecolor="black") # type: ignore
        plt.title(f"Histogram of column {i}") # type: ignore
        plt.xlabel("Duration[microseconds]") # type: ignore
        plt.ylabel("Frequency") # type: ignore
        plt.savefig(f"{prefix}_hist_col_{i}.png") # type: ignore
        plt.close()

def create_plot(x: NDArray[np.uint64 | np.float64], y: NDArray[np.uint64 | np.float64], prefix : str):
    plt.figure(figsize=(6, 4)) # type: ignore
    plt.plot(x, y)# type: ignore
    plt.xlabel("iteration")# type: ignore
    plt.ylabel("wave duration[microseconds]")# type: ignore
    plt.grid(True)# type: ignore
    plt.tight_layout()# type: ignore
    plt.savefig(f"{prefix}_plot.png") # type: ignore

def main() -> None:
    parser = argparse.ArgumentParser(description="Process profiling logs")
    parser.add_argument("--log-file", help="Log file to process.")
    parser.add_argument("--output", help="Output folder.")
    parser.add_argument("--node-map", help="Node name mappings.")
    args = parser.parse_args()

    if not args.log_file:
        print(f"Log file not provided!")
        sys.exit(1)

    input_file: str = args.log_file

    temp_dir = Path("temp/")
    temp_dir.mkdir(parents=True, exist_ok=True)
    
    graphs_dir = Path("temp/graphs")
    graphs_dir.mkdir(parents=True, exist_ok=True)

    w_array, j_array = parse_file(input_file)
 
    stats, task_stats, thread_stats = compute_job_stats(j_array)
    print("Job statitics calculated")

    w_stats = compute_wave_stats(w_array)

    stats["wave_stats"] = w_stats

    output_path = "temp/"
    if args.output:
        output_path = args.output

    if args.node_map:
        node_map = load_node_map(args.node_map)

        for task_id in node_map:
            if task_id in stats["task_stats"]:
                current_task_stats = stats["task_stats"].pop(task_id)
                stats["task_stats"][node_map[task_id]] = current_task_stats



    save_stats_to_yaml(stats, output_path+"stats.yaml")
    print("Statistics written to j_stats.yaml")

    for task_id in task_stats:
        create_histograms(task_stats[task_id], output_path+"graphs/task_"+str(int(task_id)))

    for thread_id in thread_stats:
        create_histograms(thread_stats[thread_id], output_path+"graphs/thread_"+str(int(thread_id)))

    create_histograms(j_array[:, [JobColumns.SCHED_DURATION]], output_path+"graphs/sched")
    create_histograms(j_array[:, [JobColumns.RUN_DURATION]], output_path+"graphs/run")
    create_histograms(j_array[:, [JobColumns.WAIT_DURATION]], output_path+"graphs/wait")

    create_histograms(w_array[:, [WaveColumns.DURATION]], output_path+"graphs/wave")
    create_plot(w_array[:, WaveColumns.WAVE_NUM], w_array[:, WaveColumns.DURATION], output_path+"graphs/wave_plot")
    print("Histograms created at temp/graphs/ folder")



if __name__ == "__main__":
    main()
