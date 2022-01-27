/*

SimpleJava:
1) No abstract class or method, no interface, implements.
2) public, private, protected will be ignored.
3) Class can NOT be defined in class.
4) method parameter hasn't default value.
5) ca.get().b is not available.
6) Do NOT support exceptions.

System module and user dedfined module are all in one Virtual Machine space,
that is: Only one static section, one static Variable section, one const string section.

When Reset Virtual Machine:
	1) user defined static Variable will be cleaned. 
	2) all static Variable will reset to default.
	3) const string section will be kept.


todo:
	1) exception handling?

*/

package com.crintsoft.zikiplayer

public class DebugLog
{
	public static native void print(int nValue);
	public static native void print(String strValue);

}

public class base 
{
	static int			m_nValue;
	static String				m_str;
	public base(int n) {
		m_nValue = n;
	}
	public base() { }
	public void setStr(String str) {
		m_str = str;
		DebugLog.print("base::setStr"); 
	}
}

public class child extends base
{
	public child() {
		
	}
	public void setStr(String str) { base.setStr(str); m_str = str; DebugLog.print("child::setStr"); }
}


public class TestAA
{
	static void main()
	{
		int		i = 0;
		int		k = 10;
		
		// DebugLog.print(child.m_str);
		
		child.m_str = "str";
		if (child.m_str == null)
			return;

		for (int n = 0; n < 10; n++)
		{
			DebugLog.print(n);
			// child.m_str = child.m_str + "x ";
			DebugLog.print(child.m_str);
		}
		
		int n = 0;

		while (k > 0)
		{
			i++;
			k--;
			if (i == k)
				break;
			DebugLog.print("hello");
			DebugLog.print(i);
		}
		
		for (int nn = 0; nn < 2; nn++)
		{
			for (int jj = 0; jj < 10; jj++)
			{
				if (jj == 8)
					continue;
				DebugLog.print(jj);
			}
			break;
		}
		
		base	b = new child();
		b.setStr("sss");

		DebugLog.print(b.m_str);
		DebugLog.print(b.m_nValue);

		child	c = new child();
		base	bb = c;
		c.m_str = "new ccc";
		DebugLog.print(b.m_str);
	}
	
	void Testa()
	{
	}
}


public class MyString
{
	static int TestStaticMethod() {
		// length = 5;
		if (m_nTemp > 5)
			;
		else
			m_nTemp = 9;
		TestStaticMethod();
	}
	
	void Print(int i) { }
	void SetValue(MyString s) { }
	
	int		size;
	int		length = 10;
	
	static int m_nTemp = 5;
}

public class UIObject
{
	public void onCreate() { }

	public void onDestroy() { }

	MyString GetText() { return m_s; }

	public void SetText(MyString s)
	{
		int		n1, n2 = 5;
		n1 = n2;
		int		n3;
		int		onDestroy;
		n1 + n2;

		// GetText().Print();
		
		m_s.length = 1;

		s = m_s;
	}

	public native void setVisible(boolean bVisible);

	static native MyString ClassName();
	
	MyString		m_s;

}

public class TextCtrl extends UIObject
{
public
	void SetText(MyString s)
	{
		m_s = s;
	}
	
	static void TestStitic()
	{
		TextCtrl	ctrl;
		ctrl.m_s.length = 9;
	}
	
}