
public class CaseStaticVar
{
	static int incStaticVarInMethod(int initVal)
	{
		static int		a = initVal;
		a = a + 10;
		return a;
	}

	static int TestStaticVarInMethod()
	{
		UnitTest.message("Run TestStaticVarInMethod");
	
		int		ret;
		
		ret = incStaticVarInMethod(1);
		if (ret != 11)
			UnitTest.failedMessage("incStaticVarInMethod != 11");
		
		ret = incStaticVarInMethod(1);
		UnitTest.message(ret);
		if (ret != 21)
			UnitTest.failedMessage("incStaticVarInMethod != 21");
	}

	static int m_sVar = 0;
	
	static int incStaticMemberVar(int n)
	{
		m_sVar = m_sVar + 1;
		return m_sVar;
	}

	static int TestStaticMemberVar()
	{
		UnitTest.message("Run TestStaticMemberVar");
	
		int		ret;

		ret = incStaticMemberVar(1);
		if (ret != 1)
			UnitTest.failedMessage("incStaticMemberVar != 1");

		ret = incStaticMemberVar(1);
		if (ret != 2)
			UnitTest.failedMessage("incStaticMemberVar != 2");
	}
}

public class CaseMemberVar_A
{
	public int		m_Var1 = 1;
	CaseMemberVar_A() { m_Var1 = 1; }
}

public class CaseMemberVar_B extends CaseMemberVar_A
{
	public int		m_VarB1 = 2;
}

public class CaseMemberVar
{
	public static void TestMemberVar()
	{
		CaseMemberVar_A		a;
		a = new CaseMemberVar_A();

		if (a.m_Var1 != 1)
			UnitTest.failedMessage("TestMemberVar: a.m_Var1 != 1");

		a.m_Var1 = a.m_Var1 + 5;
		if (a.m_Var1 != 6)
			UnitTest.failedMessage("TestMemberVar: a.m_Var1 != 6");
	}

	public static void TestMemberVarDerive()
	{
		CaseMemberVar_A		a;
		a = new CaseMemberVar_B();

		if (a.m_Var1 != 1)
			UnitTest.failedMessage("TestMemberVarDerive: a.m_Var1 != 1");

		CaseMemberVar_B		b = (CaseMemberVar_B)a;
		if (b.m_VarB1 != 2)
			UnitTest.failedMessage("TestMemberVarDerive: b.m_VarB1 != 2");
	}

}

//////////////////////////////////////////////////////////////////////////

public class MyException extends Exception {
	MyException(String str) {
		super(str);
	}

}

public class TestThrowException {
	void MethodThrow() throws Exception, MyException {
		throw new MyException("Throw Exception");
	}
	
	void CatchException() {
		int		sp, bp;
		sp = UnitTest.getSp();
		bp = UnitTest.getBp();

		UnitTest.message("CatchException");
		try
		{
			MethodThrow();
		}
		catch (Exception e)
		{
			e.toString();
			UnitTest.message("catch exception");
		}
		catch (MyException e)
		{
			e.toString();
			UnitTest.message("catch MyException");
		}

		if (sp != UnitTest.getSp())
			UnitTest.failedMessage("CatchException: sp != UnitTest.getSp()");
		if (bp != UnitTest.getBp())
			UnitTest.failedMessage("CatchException: bp != UnitTest.getBp()");
	}
}

public class CaseExcetpion
{
	public static void TestExceptions()
	{
		UnitTest.message("Run TestExceptions()");
		try
		{
			CaseMemberVar_A		a;
			a = new CaseMemberVar_A();

			CaseMemberVar_B		b = (CaseMemberVar_B)a;
		}
		catch (Exception e)
		{
			UnitTest.message("CaseExcetpion: " + e.toString());
		}

		UnitTest.message("Run TestThrowException()");
		TestThrowException	testE = new TestThrowException();
		testE.CatchException();
	}

}

public class CaseStack
{
	public static void TestStack()
	{
		UnitTest.message("Run TestStack()");
		
		for (int k = 0; k < 10; k++)
		{
			UnitTest.printJVMStatus();
			for (int i = 0; i < 100; i++)
			{
				MyException	myExcep = new MyException("Test my exception");
			}
		}

	}

}

public class Test
{
	static void main()
	{
		CaseStaticVar.TestStaticVarInMethod();
		CaseStaticVar.TestStaticMemberVar();
		
		CaseMemberVar.TestMemberVar();
		CaseMemberVar.TestMemberVarDerive();
		
		CaseExcetpion.TestExceptions();
		
		CaseStack.TestStack();

		int		a = 5, b = 10;
		if (a + b != 15)
			UnitTest.failedMessage("a + b != 15");
	}
}
