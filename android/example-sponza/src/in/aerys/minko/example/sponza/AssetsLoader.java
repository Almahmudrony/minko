package in.aerys.minko.example.sponza;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.res.AssetManager;
import android.util.Log;

public class AssetsLoader
{
	public static byte[] LoadAsset(String name)
	{
		Log.d("Minko", "Reading " + name);
		AssetManager manager = MainActivity.Assets;
		InputStream is;
		
		try {
			is  = manager.open(name);


			ByteArrayOutputStream buffer = new ByteArrayOutputStream();
			byte[] data = new byte[512];

			int num;
			
			while (true)
			{
				num = is.read(data);
				if (num == -1)
					break;
				buffer.write(data, 0, num);
			}
			
			return buffer.toByteArray();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
	}
}
