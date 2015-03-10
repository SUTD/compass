
import verify.*;

public class Duck
{
public int getSize()
{
  return -1;
}



public static void test333(int x)
{
  Duck d = new Duck();
  int a = d.getSize();
  Compass.static_assert(a==-1);
}


}
