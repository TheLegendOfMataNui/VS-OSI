using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSIProject.Language.OSIAssembly
{
    public static class Language
    {
        public static readonly string[] BlueKeywords = new string[] { "begin", "end" };
        public static readonly string[] BlockKeywords = new string[] { "metadata", "strings", "globals", "symbols", "sources", "functions", "classes", "class", "subroutine" };
    }

    public enum TokenType
    {
        Invalid,
        Whitespace,
        Keyword, // begin, end, metadata, strings, sources, functions, classes, property, method, class
        Comment, // ; stuff
        Comma, // ,
        NumberLiteral, // -1049
        StringLiteral, // "thing"
    }

    public class Token
    {
        public string Content { get; }
        public TokenType Type { get; }
        public int StartIndex { get; }
        public int Length { get; }

        public Token(string content, TokenType type, int startIndex, int length)
        {
            this.Content = content;
            this.Type = type;
            this.StartIndex = startIndex;
            this.Length = length;
        }

        public override string ToString()
        {
            return Type.ToString() + ": '" + Content + "'";
        }
    }

    public static class Lexer
    {
        public static List<Token> Lex(string input, bool includeWhitespace = false)
        {
            // This is greatly simplified by the fact that the token type can be uniquely identified by the first character of the token.
            List<Token> results = new List<Token>();
            //string buffer = "";
            TokenType type;
            int startIndex = 0;
            int index = 0;
            bool wasEscapeSlash = false;
            while (index < input.Length)
            {
                //buffer = "";
                startIndex = index;
                char ch = input[index];
                type = ClassifyFirstCharacter(ch);
                while (index < input.Length && CanBePartOfToken(ch, type, index - startIndex))
                {
                    //buffer += ch;
                    index++;
                    if (type == TokenType.StringLiteral && !wasEscapeSlash && ch == '"' && index > startIndex + 1)
                        break;
                    wasEscapeSlash = type == TokenType.StringLiteral && ch == '\\';
                    if (index < input.Length)
                        ch = input[index];
                }

                // Tweak the Infinity and NaN Keywords to NumberLiterals
                string content = input.Substring(startIndex, index - startIndex);
                if (type == TokenType.Keyword && (content == "Infinity" || content == "NaN"))
                    type = TokenType.NumberLiteral;

                if (type != TokenType.Whitespace || includeWhitespace)
                    results.Add(new Token(content, type, startIndex, index - startIndex));
            }

            return results;
        }

        private static TokenType ClassifyFirstCharacter(char character)
        {
            if (character == '"')
                return TokenType.StringLiteral;
            else if (Char.IsLetter(character))
                return TokenType.Keyword;
            else if (Char.IsDigit(character) || character == '-' || character == '+' || character == '.')
                return TokenType.NumberLiteral;
            else if (character == ',')
                return TokenType.Comma;
            else if (character == ';')
                return TokenType.Comment;
            else if (Char.IsWhiteSpace(character))
                return TokenType.Whitespace;
            else
                return TokenType.Invalid;
        }

        private static bool CanBePartOfToken(char character, TokenType type, int position)
        {
            if (type == TokenType.Comment)
                return character != '\n'; // Everything till a line break
            else if (type == TokenType.Comma)
                return character == ',' && position == 0; // Only one character, a comma
            else if (type == TokenType.NumberLiteral)
                return Char.IsNumber(character) || Char.IsLetter(character) || character == '-' || character == '+' || character == '.' || character == 'e' || character == 'E' || character == 'x' || character == 'X' || character == 'o' || character == 'O' || character == 'b' || character == 'B';
            else if (type == TokenType.Invalid)
                return !Char.IsLetterOrDigit(character)
                    && !Char.IsWhiteSpace(character)
                    && character != ','
                    && character != ';'
                    && character != '-'
                    && character != '"';
            else if (type == TokenType.Keyword)
                return Char.IsLetter(character) || Char.IsNumber(character);
            else if (type == TokenType.StringLiteral)
                return character != '\n'; // Anything besides a newline
            else if (type == TokenType.Whitespace)
                return Char.IsWhiteSpace(character);
            else
                throw new ArgumentException("Invalid TokenType in '" + nameof(type) + "'!");
        }
    }
}
