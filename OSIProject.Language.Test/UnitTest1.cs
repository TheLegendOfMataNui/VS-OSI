using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using OSIProject.Language.OSIAssembly;

namespace OSIProject.Language.Test
{
    [TestClass]
    public class UnitTest1
    {
        private const string testFragment1 = @"; Generator: @sage-js/cli: 0.9.1
; SHA256: 40B7915E02CF2E5B51CE1ED64B829B83C39635E5724EBCC8276DE9306AC02E90

begin metadata
    version 4, 1
end ; metadata

begin strings
    string "" ""                                                                                                                                                     ; 0

    string ""\n""                                                                                                                                                    ; 1
    string ""Root/Data/Strings/""                                                                                                                                    ; 2
    string ""English.tbl""                                                                                                                                           ; 3
    string ""German.tbl""                                                                                                                                            ; 4
    string ""French.tbl""                                                                                                                                            ; 5
    string ""Danish.tbl""                                                                                                                                            ; 6
    string ""Swedish.tbl""                                                                                                                                           ; 7
    string ""Norwegian.tbl""              ";

        [TestMethod]
        public void TestMethod1()
        {
            List<OSIAssembly.Token> results = OSIAssembly.Lexer.Lex(System.IO.File.ReadAllText(@"D:\codemastrben\Documents\Projects\Modding\Bionicle\Sample Files\osi stuff\betabase.osa"));
            foreach (OSIAssembly.Token token in results)
            {
                Debug.WriteLine(token.ToString());
            }
            //VerifyTokens(results, new List<Token>());
        }

        private const string InputNumberPlain = "104020192582";
        private const string InputNumberPlainPositive = "+55410482";
        private const string InputNumberPlainNegative = "-5823";
        private const string InputNumberHex = "0x481AF9";
        private const string InputNumberHexNegative = "-0x59fF";
        private const string InputNumberHexPositive = "+0x52d";
        private const string InputNumberOct = "0o37777";
        private const string InputNumberOctNegative = "-0o512635";
        private const string InputNumberOctPositive = "+0o153623";
        private const string InputNumberBin = "0b01001010";
        private const string InputNumberBinPositive = "+0b01010010";
        private const string InputNumberBinNegative = "-0b10101010";
        private const string InputNumberInfinity = "Infinity";
        private const string InputNumberInfinityPositive = "+Infinity";
        private const string InputNumberInfinityNegative = "-Infinity";
        private const string InputNumberNaN = "NaN";
        private const string InputNumberFloat = ".06e+52";
        private const string InputNumberFloat2 = "+.09e82";
        private const string InputNumberFloat3 = "-52.173";
        private const string InputNumberFloat4 = "-382E92";
        private const string InputNumberFloat5 = ".E+532";

        [TestMethod]
        public void TestNumbers()
        {
            //List<Token> results = Lexer.Lex(CommaTest1);
            //VerifyTokens(results, new List<Token>());
            string[] numberTests = new string[]
            {
                InputNumberPlain,
                InputNumberPlainPositive,
                InputNumberPlainNegative,
                InputNumberHex,
                InputNumberHexPositive,
                InputNumberHexNegative,
                InputNumberOct,
                InputNumberOctPositive,
                InputNumberOctNegative,
                InputNumberBin,
                InputNumberBinPositive,
                InputNumberBinNegative,
                InputNumberInfinity,
                InputNumberInfinityPositive,
                InputNumberInfinityNegative,
                InputNumberNaN,
                InputNumberFloat,
                InputNumberFloat2,
                InputNumberFloat3,
                InputNumberFloat4,
                InputNumberFloat5,
            };

            foreach (string number in numberTests)
            {
                Assert.IsTrue(VerifyNumber(number), number);
            }
        }

        private bool VerifyNumber(string input)
        {
            List<Token> results = Lexer.Lex(input);
            if (results.Count != 1)
                return false;

            return results[0].Type == Token.TokenType.NumberLiteral && results[0].Content == input;
        }

        private bool VerifyTokens(List<Token> results, List<Token> reference)
        {
            bool result = true;
            const int ColumnWidth = 100;
            Debug.WriteLine("Results: " + results.Count + " items".PadRight(ColumnWidth, ' ') + "Reference: " + reference.Count + " items");
            for (int i = 0; i < (results.Count > reference.Count ? results.Count : reference.Count); i++)
            {
                string left = "   ";
                if (i < results.Count)
                    left += results[i].ToString();
                if (left.Length > ColumnWidth)
                    left = left.Substring(0, ColumnWidth - 3) + "...";
                string right = "";
                if (i < reference.Count)
                    right = reference[i].ToString();
                Debug.WriteLine(left.PadRight(ColumnWidth, ' ') + right);
            }

            return true;
        }
    }
}
