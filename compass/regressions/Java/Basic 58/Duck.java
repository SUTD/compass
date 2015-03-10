
import verify.*;

 
 class Tester {

void test(int a)
{
    int[] anArray;

    anArray = new int[10];
    anArray[0] = 1;
    anArray[1] = 2;
    Compass.static_assert(anArray[0] == 1);
    Compass.static_assert(anArray[1] == 2);
}

}