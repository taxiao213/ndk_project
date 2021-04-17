package com.taxiao.cn.apple;

import android.content.Context;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by hanqq on 2021/4/14
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class FileUtils {


    public static String copyAssetAndWrite(Context context, String fileName) {
        String filePath = null;
        if (context != null) {
            File cacheDir = context.getCacheDir();
            if (!cacheDir.exists()) cacheDir.mkdirs();
            File outFile = new File(cacheDir, fileName);
            InputStream inputStream = null;
            OutputStream outputStream = null;
            if (!outFile.exists()) {
                try {
                    inputStream = context.getAssets().open(fileName);
                    outputStream = new FileOutputStream(outFile);
                    int size = 0;
                    byte[] bytes = new byte[1024];
                    while ((size = inputStream.read(bytes)) != -1) {
                        outputStream.write(bytes, 0, size);
                    }
                    outputStream.flush();
                    inputStream.close();
                    outputStream.close();
                    filePath = outFile.getAbsolutePath();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } else {
                filePath = outFile.getAbsolutePath();
            }
        }
        return filePath;
    }

}
