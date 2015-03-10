
import verify.*;


class Point
{
  int x;
  int y;
};
 
class SwapElementsExample {

 
  public static void main(String[] args) {
	  
	  Point p = new Point();
	  int num1 = 10;
	  int num2 = 20;
	  p.x = num1;
	  p.y = num2;
	 
	  swap(p);

	 Compass.static_assert(p.x == 20);
	 Compass.static_assert(p.y == 10);
  }

    private static void swap(Point p) {
	    
	    int temp = p.x;
	    p.x = p.y;
	    p.y = temp;
	    
	    
    }
}
