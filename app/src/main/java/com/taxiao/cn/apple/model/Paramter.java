package com.taxiao.cn.apple.model;

/**
 * Created by hanqq on 2021/5/25
 * Email:yin13753884368@163.com
 * CSDN:http://blog.csdn.net/yin13753884368/article
 * Github:https://github.com/taxiao213
 */
public class Paramter {
    private String name;
    private String fileName;
    private String path;

    public Paramter(String name, String fileName, String path) {
        this.name = name;
        this.fileName = fileName;
        this.path = path;
    }

    public String getName() {
        return name;
    }

    public String getPath() {
        return path;
    }
}
