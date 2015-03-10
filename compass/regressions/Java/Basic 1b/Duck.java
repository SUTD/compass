
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
  //should fail
  Compass.static_assert(a==0);
}



}