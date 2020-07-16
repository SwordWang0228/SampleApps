#include <jni.h>#include <string>#include "AndroidLog.h"#include "gif_lib.h"#include <android/bitmap.h>#include <malloc.h>#define argb(a, r, g, b) ((((a) & 0xff) << 24)|(((b) & 0xff)<< 16)|(((g) & 0xff) << 8)| r & 0xff)/** * 定义结构体：类似于Cpp及Java中的class */typedef struct GifBean {    // 当前帧    int current_frame;    // 总帧数    int total_frame;    // 总延迟：指针类型    int *delays;} GifBean;/** * 声明绘制帧的方法 */void drawFrame(GifFileType *gifFileType, GifBean *bean, AndroidBitmapInfo bitmapInfo, void *pixels);/** * 绘制帧 */void drawFrame(GifFileType *gifFileType, GifBean *bean, AndroidBitmapInfo bitmapInfo, void *pixels) {    // LOGE("native exec drawFrame...");    SavedImage savedImage = gifFileType->SavedImages[bean->current_frame];    GifImageDesc frameInfo = savedImage.ImageDesc;    // 整幅Bitmap的首地址    int *px = (int *) pixels;    // 每一行的首地址    int *line;    // 用来解压缩的字典    ColorMapObject *colorMapObject = frameInfo.ColorMap;    px = (int *) ((char *) px + bitmapInfo.stride * frameInfo.Top);    GifByteType gifByteType;    GifColorType gifColorType;    // 先遍历行 内容的区域    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {        line = px;        // 遍历列        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {            // 索引            int pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);            // LOGE("y = %d , x = %d , pointPixel = %d \n ", y, x, pointPixel);            // 当前帧的像素数据 压缩 lzw算法            gifByteType = savedImage.RasterBits[pointPixel];            // 字典            gifColorType = colorMapObject->Colors[gifByteType];            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);        }        px = (int *) ((char *) px + bitmapInfo.stride);    }}/** * 加载gif图片 */extern "C"JNIEXPORT jlong JNICALLJava_com_zy_gifloader2_GifLoader_loadGif(JNIEnv *env, jobject thiz, jstring _path) {    // TODO: implement loadGif()    // LOGE("native loadgif run ...");    // 获取传入的路径,使用字符指针接收    const char *path = env->GetStringUTFChars(_path, 0);    // LOGE("file path = %s", path);    // gif文件    // Android giflib    int err;    // LOGE("native loadgif DGifOpenFileName run ...");    GifFileType *gifFileType = DGifOpenFileName(path, &err);    // LOGE("native loadgif DGifSlurp run ...");    DGifSlurp(gifFileType);    // LOGE("native loadgif 填充gifBean数据 run ...");    // gif 帧数 总帧数 每一帧播放的时间    // gif 相等 1 不相等 2    GifBean *gifBean = (GifBean *) malloc(sizeof(GifBean));    // 清空内存地址    memset(gifBean, 0, sizeof(GifBean));    // 填充数据    gifFileType->UserData = gifBean;    // 申请内存，并赋值    gifBean->delays = (int *) malloc(sizeof(int) * gifFileType->ImageCount);    // 释放申请的内存空间    memset(gifBean->delays, 0, sizeof(int) * gifFileType->ImageCount);    gifBean->total_frame = gifFileType->ImageCount;    gifBean->current_frame = 0;    // LOGE("native loadgif  填充gifBean->delays数据 run ...");    // 遍历每一帧 图形控制扩展块 -- 延迟时间 // Delay Time - 单位1/100秒 10ms    for (int i = 0; i < gifBean->total_frame; ++i) {        // 取出帧数据        SavedImage frame = gifFileType->SavedImages[i];        ExtensionBlock *extensionBlock;        // 找到扩展区块        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {                extensionBlock = &frame.ExtensionBlocks[j];                break;            }        }        // 如果扩展模块找到的话        if (extensionBlock) {            // 延迟时间 两个字节表示一个int Bytes[2] 高八位 Bytes[1] 低八位            int delay = (extensionBlock->Bytes[2] << 8 | extensionBlock->Bytes[1]) * 10;            // LOGE("延迟时间 = %d  ", delay);            gifBean->delays[i] = delay;        }    }    // LOGE("native loadgif  填充gifBean->delays数据 run over ...");    return (jlong) gifFileType;}extern "C"JNIEXPORT jint JNICALLJava_com_zy_gifloader2_GifLoader_getWidth(JNIEnv *env, jobject thiz, jlong _ndk_gif) {    // TODO: implement getWidth()    // LOGE("native getWidth run ...");    GifFileType *gifFileType = (GifFileType *) _ndk_gif;    return gifFileType->SWidth;}extern "C"JNIEXPORT jint JNICALLJava_com_zy_gifloader2_GifLoader_getHeight(JNIEnv *env, jobject thiz, jlong _ndk_gif) {    // TODO: implement getHeight()    // LOGE("native getHeight run ...");    GifFileType *gifFileType = (GifFileType *) _ndk_gif;    return gifFileType->SHeight;}extern "C"JNIEXPORT jlong JNICALLJava_com_zy_gifloader2_GifLoader_updateFrame(JNIEnv *env, jobject thiz, jlong _ndk_gif, jobject _bitmap) {    // LOGE("native updateFrame run ...");    GifFileType *gifFileType = (GifFileType *) _ndk_gif;    GifBean *gifBean = (GifBean *) gifFileType->UserData;    // 绘制    // bitmap.    // 代表Bitmap信息    AndroidBitmapInfo bitmapInfo;    // LOGE("native AndroidBitmap_getInfo");    // uint32_t stride: 每一行的像素    AndroidBitmap_getInfo(env, _bitmap, &bitmapInfo);    // bitmap 转换缓冲区 byte[]    void *pixels;    // LOGE("native AndroidBitmap_lockPixels");    // 这里会给picxels填充数据    AndroidBitmap_lockPixels(env, _bitmap, &pixels);    // LOGE("native pre exec drawFrame");    // 绘制    drawFrame(gifFileType, gifBean, bitmapInfo, pixels);    // LOGE("native pre exec drawFrame over...");    gifBean->current_frame += 1;    // LOGE("当前帧 = %d ", gifBean->current_frame);    // 防止越界    if (gifBean->current_frame >= gifBean->total_frame - 1) {        gifBean->current_frame = 0;    }    // LOGE("native AndroidBitmap_unlockPixels");    AndroidBitmap_unlockPixels(env, _bitmap);    return gifBean->delays[gifBean->current_frame];}extern "C"JNIEXPORT jint JNICALLJava_com_zy_gifloader2_MainActivity_sum(JNIEnv *env, jobject thiz, jint a, jint b) {    // LOGE("native sum run ...");    return a + b;}