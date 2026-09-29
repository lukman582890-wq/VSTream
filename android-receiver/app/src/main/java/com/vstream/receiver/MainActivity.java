package com.vstream.receiver;

import android.app.Activity;
import android.content.Intent;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioTrack;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.URI;
import java.util.Arrays;
import java.util.concurrent.ArrayBlockingQueue;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("vstream_decoder");
    }

    private static final String GROUP = "239.255.77.77";
    private static final int PORT = 45822;

    private EditText urlEdit;
    private TextView status;
    private Receiver receiver;

    private static native void nativeInit(String sessionId);
    private static native void nativeReset();
    private static native short[] nativeDecode(byte[] packet);

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        buildUi();

        Uri incoming = getIntent().getData();
        if (incoming != null && "vstream".equalsIgnoreCase(incoming.getScheme())) {
            String url = incoming.getQueryParameter("url");
            if (url != null) urlEdit.setText(Uri.decode(url));
        }
    }

    private void buildUi() {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(32, 48, 32, 32);

        TextView title = new TextView(this);
        title.setText("VSTream Receiver");
        title.setTextSize(26);
        root.addView(title);

        urlEdit = new EditText(this);
        urlEdit.setHint("http://192.168.x.x:45821/s/session");
        root.addView(urlEdit);

        Button connect = new Button(this);
        connect.setText("CONNECT & PLAY");
        root.addView(connect);

        Button stop = new Button(this);
        stop.setText("STOP");
        root.addView(stop);

        status = new TextView(this);
        status.setText("Idle");
        status.setTextSize(16);
        root.addView(status);

        connect.setOnClickListener(v -> connect());
        stop.setOnClickListener(v -> stop());

        setContentView(root);
    }

    private void connect() {
        stop();
        final String text = urlEdit.getText().toString().trim();
        new Thread(() -> {
            try {
                URI uri = new URI(text);
                String path = uri.getPath();
                if (uri.getHost() == null || path == null || !path.startsWith("/s/"))
                    throw new IllegalArgumentException("Invalid VSTream URL");
                String sessionId = path.substring(3);
                if (sessionId.length() != 8)
                    throw new IllegalArgumentException("Invalid session ID");

                runOnUiThread(() -> status.setText("Connecting to " + uri.getHost() + "..."));
                receiver = new Receiver(sessionId);
                receiver.start();
                runOnUiThread(() -> status.setText("LISTENING • Opus UDP"));
            } catch (Exception e) {
                runOnUiThread(() -> status.setText("ERROR: " + e.getMessage()));
            }
        }).start();
    }

    private void stop() {
        if (receiver != null) receiver.stop();
        receiver = null;
        nativeReset();
    }

    private final class Receiver {
        private final String sessionId;
        private volatile boolean running;
        private MulticastSocket socket;
        private WifiManager.MulticastLock multicastLock;
        private AudioTrack audioTrack;
        private Thread networkThread;
        private Thread playbackThread;
        private final ArrayBlockingQueue<short[]> queue = new ArrayBlockingQueue<>(6);

        Receiver(String sessionId) { this.sessionId = sessionId; }

        void start() throws Exception {
            nativeInit(sessionId);

            WifiManager wifi = (WifiManager) getApplicationContext().getSystemService(WIFI_SERVICE);
            multicastLock = wifi.createMulticastLock("VSTream");
            multicastLock.setReferenceCounted(false);
            multicastLock.acquire();

            socket = new MulticastSocket(PORT);
            socket.setReuseAddress(true);
            socket.setSoTimeout(250);
            socket.joinGroup(InetAddress.getByName(GROUP));

            int min = AudioTrack.getMinBufferSize(
                48000,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);
            audioTrack = new AudioTrack.Builder()
                .setAudioAttributes(new AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_MEDIA)
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build())
                .setAudioFormat(new AudioFormat.Builder()
                    .setSampleRate(48000)
                    .setChannelMask(AudioFormat.CHANNEL_OUT_STEREO)
                    .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                    .build())
                .setBufferSizeInBytes(Math.max(min * 2, 19200))
                .setTransferMode(AudioTrack.MODE_STREAM)
                .build();

            audioTrack.play();
            running = true;

            networkThread = new Thread(this::networkLoop, "VSTream UDP");
            playbackThread = new Thread(this::playbackLoop, "VSTream Audio");
            networkThread.start();
            playbackThread.start();
        }

        private void networkLoop() {
            byte[] buffer = new byte[4096];
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            while (running) {
                try {
                    packet.setLength(buffer.length);
                    socket.receive(packet);
                    byte[] copy = Arrays.copyOf(packet.getData(), packet.getLength());
                    short[] pcm = nativeDecode(copy);
                    if (pcm != null) {
                        if (!queue.offer(pcm)) {
                            queue.poll();
                            queue.offer(pcm);
                        }
                    }
                } catch (Exception ignored) {
                }
            }
        }

        private void playbackLoop() {
            while (running) {
                try {
                    short[] pcm = queue.take();
                    int offset = 0;
                    while (running && offset < pcm.length) {
                        int written = audioTrack.write(pcm, offset, pcm.length - offset, AudioTrack.WRITE_BLOCKING);
                        if (written <= 0) break;
                        offset += written;
                    }
                } catch (InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                    break;
                }
            }
        }

        void stop() {
            running = false;
            if (networkThread != null) networkThread.interrupt();
            if (playbackThread != null) playbackThread.interrupt();

            try {
                if (socket != null) {
                    socket.leaveGroup(InetAddress.getByName(GROUP));
                    socket.close();
                }
            } catch (Exception ignored) {}

            if (audioTrack != null) {
                try { audioTrack.stop(); } catch (Exception ignored) {}
                audioTrack.release();
            }
            if (multicastLock != null && multicastLock.isHeld())
                multicastLock.release();

            queue.clear();
            socket = null;
            audioTrack = null;
        }
    }
}
