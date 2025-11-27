import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter
import matplotlib.ticker as ticker
from typing import Dict, Any
import argparse
import re
import warnings
import os
warnings.filterwarnings("ignore", category=UserWarning, module="matplotlib.font_manager")

# -------------------------- 全局样式与配置 --------------------------
EFC_STYLES = {
    64: {"color": "#1f77b4", "marker": "o", "markersize": 5, "linewidth": 1.8, "label": "efc=64"},
    128: {"color": "#ff7f0e", "marker": "^", "markersize": 5, "linewidth": 1.8, "label": "efc=128"},
    256: {"color": "#2ca02c", "marker": "s", "markersize": 5, "linewidth": 1.8, "label": "efc=256"},
    512: {"color": "#d62728", "marker": "d", "markersize": 5, "linewidth": 1.8, "label": "efc=512"},
    1024: {"color": "#9467bd", "marker": "p", "markersize": 5, "linewidth": 1.8, "label": "efc=1024"}
}

EF_CONSTRUCTION_LIST = [64, 128, 256, 512, 1024]
CSV_SUFFIX = "_smoothed.csv"
SAVE_DIR = "./"

def create_config(dataset: str, distance: str, degree: int, k: int, dataset_path: str = "/data") -> Dict[str, Any]:
    if distance.lower() == "l2":
        actual_distance = "L2"
    elif distance.lower() == "angular":
        actual_distance = "angular"
    else:
        raise ValueError(f"Unsupported distance: {distance}")

    # 动态设置 recall_min：根据 k 值调整！
    if k == 1:
        base_recall_min = 0.05   # ← 关键：k=1 时用很低的阈值
    elif k == 10:
        base_recall_min = 0.3
    else:  # k == 100
        base_recall_min = 0.5

    config = {
        "dataset": dataset,
        "distance": actual_distance,
        "degree": degree,
        "k": k,
        "path_prefix": os.path.join(dataset_path, dataset),
        "recall_min": base_recall_min,          # ← 使用动态阈值
        "recall_ylim": (base_recall_min, 1.0),  # ← y轴下限也同步
        "title": f"{dataset.upper()}-{actual_distance.upper()}, $K={k}$"
    }

    return config

def get_csv_path(config: Dict[str, Any], efc: int, gt_type: str) -> str:
    filename = f"symqg_{config['dataset']}_deg{config['degree']}_ef{efc}_{config['distance']}_{gt_type}{CSV_SUFFIX}"
    return os.path.join(config["path_prefix"], filename)

def get_gt_type(k: int) -> str:
    if k == 1:
        return "gt1"
    elif k == 10:
        return "gt10"
    elif k == 100:
        return "gt100"
    else:
        return f"gt{k}"

# 在 read_data 中，先读全部数据，再动态设 recall_min
def read_data(config: Dict[str, Any]) -> Dict[str, Any]:
    data_dict = {}
    gt_type = get_gt_type(config["k"])
    recall_col = f"recall@{config['k']}"

    print(f"\n=== 读取数据：{config['dataset']} | deg={config['degree']} | k={config['k']} ===")

    all_recalls = []  # 收集所有 recall 值

    for efc in EF_CONSTRUCTION_LIST:
        if efc < 2 * config["degree"]:
            continue

        csv_path = get_csv_path(config, efc, gt_type)
        try:
            df = pd.read_csv(csv_path, usecols=["ef", recall_col, "qps"])
            df["recall"] = df[recall_col] / 100.0
            all_recalls.extend(df["recall"].tolist())
        except Exception as e:
            continue

    # 动态设置 recall_min：取所有 recall 的最小值，但不低于 0.01
    if all_recalls:
        dynamic_min = max(0.01, min(all_recalls) - 0.01)  # 留一点余量
        config["recall_min"] = dynamic_min
        config["recall_ylim"] = (dynamic_min, min(1.0, max(all_recalls) + 0.02))
        print(f"  动态 recall_min 设为: {dynamic_min:.3f}")
    else:
        raise ValueError("无任何有效数据")

    # 再次遍历，应用新的 recall_min
    for efc in EF_CONSTRUCTION_LIST:
        if efc < 2 * config["degree"]:
            continue
        csv_path = get_csv_path(config, efc, gt_type)
        try:
            df = pd.read_csv(csv_path, usecols=["ef", recall_col, "qps"])
            df["recall"] = df[recall_col] / 100.0
            df = df[df["recall"] >= config["recall_min"]].reset_index(drop=True)
            if len(df) > 0:
                data_dict[efc] = {
                    "ef": df["ef"].values,
                    "recall": df["recall"].values,
                    "qps": df["qps"].values
                }
        except:
            pass

    return data_dict



def plot_recall_vs_ef(data_dict: Dict[str, Any], config: Dict[str, Any]):
    fig, ax = plt.subplots(figsize=(12, 6))
    
    all_ef = []
    for efc, data in data_dict.items():
        if len(data["ef"]) == 0:
            continue
        style = EFC_STYLES[efc]
        ax.plot(
            data["ef"], data["recall"],
            color=style["color"], marker=style["marker"],
            markersize=style["markersize"], linewidth=style["linewidth"],
            label=style["label"]
        )
        all_ef.extend(data["ef"])
    
    ax.set_xlabel("ef Parameter (Query Beam Size)", fontsize=12, fontweight="bold")
    ax.set_ylabel(f"Recall@{config['k']}", fontsize=12, fontweight="bold")
    ax.set_title(f"Recall vs ef ({config['title']})", fontsize=14, fontweight="bold", pad=20)
    
    if all_ef:
        ax.set_xlim(min(all_ef) - 5, max(all_ef) + 5)
    ax.set_ylim(config["recall_ylim"])
    ax.yaxis.set_major_formatter(FuncFormatter(lambda x, _: f"{x:.3f}"))
    ax.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
    
    ax.grid(True, linestyle="--", alpha=0.3, color="#999999")
    ax.legend(loc="center left", bbox_to_anchor=(1.05, 0.5), fontsize=9, framealpha=0.9)
    
    save_path = (f"{SAVE_DIR}{config['dataset']}_deg{config['degree']}_k{config['k']}"
                 f"_{config['distance']}_recall_vs_ef.png")
    plt.tight_layout()
    plt.savefig(save_path, dpi=300, bbox_inches="tight", facecolor="white", edgecolor="none")
    print(f"✓ Recall vs ef 图表保存：{save_path}")
    plt.close()

def plot_recall_vs_qps(data_dict: dict, config: dict):
    fig, ax = plt.subplots(figsize=(12, 6))
    
    for efc, data in data_dict.items():
        if len(data["recall"]) == 0:
            continue
        style = EFC_STYLES[efc]
        
        ax.plot(
            data["recall"], data["qps"],
            color=style["color"], marker=style["marker"],
            markersize=style["markersize"], linewidth=style["linewidth"],
            linestyle=style.get("linestyle", "-"),
            label=style["label"]
        )
        
    ax.set_xlabel(f"Recall@{config['k']}", fontsize=12, fontweight="bold")
    ax.set_ylabel("QPS (Queries Per Second)", fontsize=12, fontweight="bold")
    ax.set_title(f"Recall vs QPS ({config['title']})", fontsize=14, fontweight="bold", pad=20)
    
    ax.xaxis.set_major_formatter(FuncFormatter(lambda x, _: f"{x:.3f}"))
    ax.grid(True, linestyle="--", alpha=0.3, color="#999999")
    
    ax.legend(loc="center left", bbox_to_anchor=(1.05, 0.5), fontsize=9, framealpha=0.9)
    
    save_path = f"{SAVE_DIR}{config['dataset']}_deg{config['degree']}_k{config['k']}_{config['distance']}_recall_vs_qps.png"
    plt.tight_layout()
    plt.savefig(save_path, dpi=300, bbox_inches="tight", facecolor="white", edgecolor="none")
    print(f"✓ Recall vs QPS 图表保存：{save_path}")
    plt.close()

def main():
    parser = argparse.ArgumentParser(description="绘制ANN搜索性能图表")
    parser.add_argument("--dataset", required=True, 
                       choices=["glove2.2m", "sift10M", "gist", "tiny5m"],
                       help="数据集名称")
    parser.add_argument("--distance", required=True,
                       choices=["L2", "angular"],
                       help="距离类型（和Shell脚本一致）")
    parser.add_argument("--degree", type=int, required=True,
                       choices=[32, 64],
                       help="degree值")
    parser.add_argument("--k", type=int, required=True,
                       choices=[1, 10, 100],
                       help="K值")
    parser.add_argument("--dataset-path", default="/data",
                       help="数据集路径（默认：/data）")
    
    args = parser.parse_args()
    
    try:
        config = create_config(args.dataset, args.distance, args.degree, args.k, args.dataset_path)
        data_dict = read_data(config)
        plot_recall_vs_ef(data_dict, config)
        plot_recall_vs_qps(data_dict, config)
        
        print(f"\n✅ 图表生成完成！数据集：{args.dataset}, 距离：{args.distance}, "
              f"degree：{args.degree}, K：{args.k}")
        
    except Exception as e:
        print(f"\n❌ 错误：{str(e)}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())