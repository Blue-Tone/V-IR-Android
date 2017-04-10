package com.example.admin.vir;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.view.View;

import com.example.admin.vir.R.id;

import static com.example.admin.vir.R.id.sample_text;

public class MainActivity extends AppCompatActivity {

    TextView tv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        tv = (TextView) findViewById(sample_text);
        tv.setText(stringFromJNI());

        Button button = (Button) findViewById(id.button);
        // ボタンがクリックされた時に呼び出されるコールバックリスナーを登録します
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // ボタンがクリックされた時に呼び出されます
                Button button = (Button) v;
//                Toast.makeText(ButtonSampleActivity.this, "onClick()",
//                        Toast.LENGTH_SHORT).show();
                tv.setText(tv.getText() + "\n" + "push");
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }
}
