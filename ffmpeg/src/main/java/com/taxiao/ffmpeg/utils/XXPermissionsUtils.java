package com.taxiao.ffmpeg.utils;

import android.Manifest;
import android.app.Activity;
import android.content.Context;

import com.hjq.permissions.OnPermission;
import com.hjq.permissions.XXPermissions;

import java.util.List;

/**
 * 权限获取工具类
 * Created by hanqq on 2020/06/30
 */
public class XXPermissionsUtils {

    private static volatile XXPermissionsUtils xxPermissionsUtils;
    public Context context;
    /*读写sd卡权限*/
    private String[] readOrWritePermission = new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE};
    /*拍照权限*/
    private String[] cameraPermission = new String[]{Manifest.permission.CAMERA};
    /*录音权限*/
    private String[] recordPermission = new String[]{Manifest.permission.RECORD_AUDIO, Manifest.permission.MODIFY_AUDIO_SETTINGS};
    /*获取指纹信息权限*/
    private String[] fingerprintPermission = new String[]{Manifest.permission.USE_BIOMETRIC, Manifest.permission.USE_FINGERPRINT};
    /*拨打电话权限*/
    private String[] callPhonePermission = new String[]{Manifest.permission.CALL_PHONE};
    /*读取手机状态权限*/
    private String[] readPhonePermission = new String[]{Manifest.permission.READ_PHONE_STATE};
    /*读取手机位置状态权限*/
    private String[] locationPermission = new String[]{Manifest.permission.ACCESS_COARSE_LOCATION, Manifest.permission.ACCESS_FINE_LOCATION};
    /*弹框权限*/
    private String[] systemAlertPermission = new String[]{Manifest.permission.SYSTEM_ALERT_WINDOW};

    public XXPermissionsUtils() {

    }

    public static XXPermissionsUtils getInstances() {
        if (xxPermissionsUtils == null) {
            synchronized (XXPermissionsUtils.class) {
                if (xxPermissionsUtils == null) {
                    xxPermissionsUtils = new XXPermissionsUtils();
                }
            }
        }
        return xxPermissionsUtils;
    }

    /**
     * 拍照权限
     */
    public void hasCameraPermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, cameraPermission, readOrWritePermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, cameraPermission, readOrWritePermission);
        }
    }

    /**
     * 读写sd卡权限
     */
    public void hasReadAndWritePermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, readOrWritePermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, readOrWritePermission);
        }
    }

    /**
     * 录音权限
     */
    public void hasRecordPermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, recordPermission, readOrWritePermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, recordPermission, readOrWritePermission);
        }
    }

    /**
     * 指纹权限
     */
    public void hasFingerprintPermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, fingerprintPermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, fingerprintPermission);
        }
    }

    /**
     * 是否有录音权限
     */
    public boolean hasRecordPermission(Context context) {
        if (XXPermissions.isHasPermission(context, recordPermission)) {
            this.context = null;
            return true;
        }
        return false;
    }

    /**
     * 拨打电话权限
     */
    public void hasCallPhonePermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, callPhonePermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, callPhonePermission);
        }
    }

    /**
     * 读取手机状态权限
     */
    public void hasReadPhonePermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, readPhonePermission, readOrWritePermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, readPhonePermission, readOrWritePermission);
        }
    }

    /**
     * 弹框权限
     */
    public void hasSystemAlertPermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, systemAlertPermission, readOrWritePermission)) {
            booleanFunction.action(true);
            this.context = null;
        } else {
            requestPermission(booleanFunction, systemAlertPermission, readOrWritePermission);
        }
    }

    /**
     * 手机通话读取必要状态权限
     */
    public void hasCallPermission(Function<Boolean> booleanFunction, Context context) {
        this.context = context;
        if (XXPermissions.isHasPermission(context, readPhonePermission, readOrWritePermission, cameraPermission, recordPermission, locationPermission, systemAlertPermission)) {
            booleanFunction.action(true);
        } else {
            requestPermission(booleanFunction, readPhonePermission, readOrWritePermission, cameraPermission, recordPermission, locationPermission, systemAlertPermission);
        }
    }

    private void requestPermission(final Function<Boolean> booleanFunction, String[]... permission) {
        Activity context = (Activity) this.context;
        if (context == null || context.isDestroyed()) return;
        XXPermissions.with(context)
                //.constantRequest() //可设置被拒绝后继续申请，直到用户授权或者永久拒绝
                //.permission(Permission.SYSTEM_ALERT_WINDOW, Permission.REQUEST_INSTALL_PACKAGES) //支持请求6.0悬浮窗权限8.0请求安装权限
                .permission(permission) //不指定权限则自动获取清单中的危险权限
                .request(new OnPermission() {
                    @Override
                    public void hasPermission(List<String> granted, boolean isAll) {
                        if (isAll) {
                            booleanFunction.action(true);
                        } else {
                            booleanFunction.action(false);
                        }
                        XXPermissionsUtils.this.context = null;
                    }

                    @Override
                    public void noPermission(List<String> denied, boolean quick) {
                        if (quick) {
//                            ToastUtils.show(XXPermissionsUtils.this.context.getString(R.string.permissions_refuse_authorization));
                            XXPermissions.gotoPermissionSettings(XXPermissionsUtils.this.context);
                        } else {
//                            ToastUtils.show(XXPermissionsUtils.this.context.getString(R.string.permissions_rror));
                        }
                        booleanFunction.action(false);
                        XXPermissionsUtils.this.context = null;
                    }
                });
    }
}
