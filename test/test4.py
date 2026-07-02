import os
import sys
from ctypes import *
import wave
import subprocess

# 配置资源文件和动态库路径
RES_PATH_XIAOYAN = "/home/mowen/Desktop/Linux_aisound_exp1227_f3105644/bin/msc/res/tts/xiaoyan.jet"
RES_PATH_COMMON = "/home/mowen/Desktop/Linux_aisound_exp1227_f3105644/bin/msc/res/tts/common.jet"
LIB_PATH = "/home/mowen/Desktop/Linux_aisound_exp1227_f3105644/libs/x64/libmsc.so"

def check_file(path):
    if not os.path.exists(path):
        print(f"错误：文件不存在 -> {path}")
        return False
    return True

def play_wav_system(output_wav):
    """
    使用系统命令播放 WAV 文件
    Linux 上推荐使用 aplay 或 paplay
    """
    try:
        subprocess.run(["aplay", output_wav], check=True)
    except Exception as e:
        print(f"播放音频失败: {e}")

def speak(text, output_wav="tts_sample.wav", appid="f3105644"):
    """
    离线TTS合成并播放
    参数：
        text: 待合成文本
        output_wav: 输出wav文件名
        appid: 科大讯飞AppID
    """
    # 1️⃣ 检查资源文件
    if not (check_file(RES_PATH_XIAOYAN) and check_file(RES_PATH_COMMON)):
        return -1

    # 2️⃣ 加载动态库
    try:
        msc = cdll.LoadLibrary(LIB_PATH)
    except Exception as e:
        print(f"错误：动态库加载失败 -> {LIB_PATH}")
        print(str(e))
        return -1

    if not hasattr(msc, "MSPLogin") or not hasattr(msc, "QTTSSessionBegin"):
        print("错误：动态库接口未找到，请确认动态库版本正确")
        return -1

    # 3️⃣ 登录
    login_params = f"appid={appid}, work_dir=."
    ret = msc.MSPLogin(None, None, login_params.encode('utf-8'))
    if ret != 0:
        print(f"错误：MSPLogin failed, error code: {ret}")
        return ret
    print("✅ 登录成功")

    # 4️⃣ 构建session参数
    session_params = (
        f"engine_type=local,voice_name=xiaoyan,text_encoding=UTF8,"
        f"tts_res_path=fo|{RES_PATH_XIAOYAN};fo|{RES_PATH_COMMON},"
        "sample_rate=16000,speed=50,volume=50,pitch=50,rdn=2"
    )

    # 5️⃣ 开始TTS合成
    ret = c_int(-1)
    msc.QTTSSessionBegin.argtypes = [c_char_p, POINTER(c_int)]
    msc.QTTSSessionBegin.restype = c_char_p
    session_id = msc.QTTSSessionBegin(session_params.encode('utf-8'), byref(ret))
    if ret.value != 0:
        print(f"错误：QTTSSessionBegin failed, error code: {ret.value}")
        msc.MSPLogout()
        return ret.value

    msc.QTTSTextPut.argtypes = [c_char_p, c_char_p, c_uint, c_void_p]
    msc.QTTSTextPut.restype = c_int
    ret_val = msc.QTTSTextPut(session_id, text.encode('utf-8'), len(text.encode('utf-8')), None)
    if ret_val != 0:
        print(f"错误：QTTSTextPut failed, error code: {ret_val}")
        msc.QTTSSessionEnd(session_id, b"TextPutError")
        msc.MSPLogout()
        return ret_val

    print("正在合成 ...")
    wav_file = wave.open(output_wav, 'wb')
    wav_file.setnchannels(1)
    wav_file.setsampwidth(2)
    wav_file.setframerate(16000)

    synth_status = c_int(1)
    audio_len = c_uint(0)
    msc.QTTSAudioGet.argtypes = [c_char_p, POINTER(c_uint), POINTER(c_int), POINTER(c_int)]
    msc.QTTSAudioGet.restype = c_void_p

    while True:
        data_ptr = msc.QTTSAudioGet(session_id, byref(audio_len), byref(synth_status), byref(ret))
        if ret.value != 0:
            print(f"错误：QTTSAudioGet failed, error code: {ret.value}")
            msc.QTTSSessionEnd(session_id, b"AudioGetError")
            wav_file.close()
            msc.MSPLogout()
            return ret.value
        if data_ptr:
            audio_data = cast(data_ptr, POINTER(c_char * audio_len.value)).contents
            wav_file.writeframes(audio_data)
        if synth_status.value == 2:  # MSP_TTS_FLAG_DATA_END
            break

    wav_file.close()
    msc.QTTSSessionEnd(session_id, b"Normal")
    msc.MSPLogout()
    print(f"✅ 合成完毕，已生成文件：{output_wav}")

    # 6️⃣ 使用系统命令播放 WAV 文件
    play_wav_system(output_wav)

    return 0

# 主函数示例
if __name__ == "__main__":
    text = "科大讯飞离线语音合成示例测试，文本可以随意修改。"
    speak(text)

