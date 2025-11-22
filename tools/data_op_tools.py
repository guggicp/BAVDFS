import numpy as np
from typing import List

def ivecs_read(filename):
    print(f"Reading File - {filename}")
    with open(filename, 'rb') as f:
        # 读取所有数据
        data = np.fromfile(f, dtype='int32')
    # 解析第一个向量的维度
    d = data[0]
    # 验证总长度是否为 (d+1) 的倍数（确保格式正确）
    if len(data) % (d + 1) != 0:
        raise ValueError(f"Invalid .ivecs file: total length {len(data)} is not a multiple of (d+1)={d+1}")
    # 拆分并返回近邻索引
    return data.reshape(-1, d + 1)[:, 1:].copy()

def fvecs_read(filename):
    return ivecs_read(filename).view('float32')

def ivecs_write(filename, m):
    print(f"Writing File - {filename}")
    n, d = m.shape
    m1 = np.empty((n, d + 1), dtype='int32')
    m1[:, 0] = d
    m1[:, 1:] = m
    m1.tofile(filename)
    print(f"\t{filename} wrote")

def fvecs_write(filename, m):
    m = m.astype('float32')
    ivecs_write(filename, m.view('int32'))

def read_fbin_metadata(filename):
    """Read only the metadata (number of vectors and dimensions) from an fbin file"""
    with open(filename, "rb") as f:
        nvecs, dim = np.fromfile(f, count=2, dtype=np.int32)
    return nvecs, dim

def read_fbin_batch(filename, start_idx, batch_size, dim):
    """Read a batch of vectors from an fbin file"""
    with open(filename, "rb") as f:
        offset = 8 + start_idx * dim * 4  # 8 bytes for header, 4 bytes per float32
        f.seek(offset)
        
        vectors = np.fromfile(f, count=batch_size * dim, dtype=np.float32)
        if len(vectors) == 0:
            return None
        
        return vectors.reshape(-1, dim)

def read_fbin(filename, start_idx=0, chunk_size=None):
    """ Read *.fbin file that contains float32 vectors
    Args:
        :param filename (str): path to *.fbin file
        :param start_idx (int): start reading vectors from this index
        :param chunk_size (int): number of vectors to read.
                                 If None, read all vectors
    Returns:
        Array of float32 vectors (numpy.ndarray)
    """
    with open(filename, "rb") as f:
        nvecs, dim = np.fromfile(f, count=2, dtype=np.int32)
        print(f'reading from {filename}, nvecs={nvecs}, dim={dim}')
        nvecs = (nvecs - start_idx) if chunk_size is None else chunk_size
        arr = np.fromfile(f, count=nvecs * dim, dtype=np.float32,
                          offset=start_idx * 4 * dim)
    return arr.reshape(nvecs, dim)

def read_ibin(filename, start_idx=0, chunk_size=None):
    """ Read *.ibin file that contains int32 vectors
    Args:
        :param filename (str): path to *.ibin file
        :param start_idx (int): start reading vectors from this index
        :param chunk_size (int): number of vectors to read.
                                 If None, read all vectors
    Returns:
        Array of int32 vectors (numpy.ndarray)
    """
    with open(filename, "rb") as f:
        nvecs, dim = np.fromfile(f, count=2, dtype=np.int32)
        print(f'reading from {filename}, nvecs={nvecs}, dim={dim}')
        nvecs = (nvecs - start_idx) if chunk_size is None else chunk_size
        arr = np.fromfile(f, count=nvecs * dim, dtype=np.int32,
                          offset=start_idx * 4 * dim)
    return arr.reshape(nvecs, dim)

def write_ibin(filename, vecs):
    """写入int32格式索引文件"""
    with open(filename, "wb") as f:
        nvecs, dim = vecs.shape
        f.write(np.array([nvecs, dim], dtype=np.int32).tobytes())
        vecs.astype(np.int32).tofile(f)

def write_fbin(filename, vecs):
    """写入float32格式距离文件"""
    with open(filename, "wb") as f:
        nvecs, dim = vecs.shape
        f.write(np.array([nvecs, dim], dtype=np.int32).tobytes())
        vecs.astype(np.float32).tofile(f)

def load_gt(ibin_path: str) -> List[np.ndarray]:
    with open(ibin_path, 'rb') as f:
        # 读取头部（nvecs 和 dim）
        nvecs = np.frombuffer(f.read(4), dtype=np.int32)[0]
        dim = np.frombuffer(f.read(4), dtype=np.int32)[0]
        
        if nvecs <= 0 or dim <= 0:
            raise RuntimeError(f"Invalid .ibin header: nvecs={nvecs}, dim={dim}")
        
        out = []
        for i in range(nvecs):
            # 关键修复：用 itemsize 获取实际字节数（int32 固定为 4 字节）
            expected_size = dim * np.dtype(np.int32).itemsize  # 等价于 dim * 4
            data_bytes = f.read(expected_size)
            
            if len(data_bytes) != expected_size:
                raise RuntimeError(f"File truncated while reading vectors: query index={i}")
            
            vec = np.frombuffer(data_bytes, dtype=np.int32)
            out.append(vec)
    
    print(f"[Info] Loaded {nvecs} groundtruth vectors, dim={dim}")
    return out


def fvec2fbin(fvecs_path: str, fbin_path: str):
    """Convert fvecs file to fbin file"""
    print(f"Converting {fvecs_path} to {fbin_path}")
    vecs = fvecs_read(fvecs_path)
    write_fbin(fbin_path, vecs)
    print(f"Conversion complete: {fvecs_path} -> {fbin_path}")