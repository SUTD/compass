import java.util.GregorianCalendar;
import verify.*;



public class Test extends GregorianCalendar
{
  String ca;
  public Test()
  {
    ca = "hello";

  }
  void setsomething(long time, String str)
  {
    this.time = 12;
    this.isTimeSet = false;
    ca = "calendar";
    Compass.static_assert(this.time == 12);
  }
}