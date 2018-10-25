using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSIProject.Language.OSIAssembly
{
    public class Token
    {
        public enum TokenType
        {
            Invalid,
            Whitespace,
            Keyword, // begin, end, metadata, strings, sources, functions, classes, property, method, class
            Comment, // ; stuff
            Comma, // ,
            IntegerLiteral, // -1049
            StringLiteral, // "thing"
        }

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
            string buffer = "";
            Token.TokenType type;
            int startIndex = 0;
            int index = 0;
            while (index < input.Length)
            {
                buffer = "";
                startIndex = index;
                char ch = input[index];
                type = ClassifyFirstCharacter(ch);
                while (index < input.Length && CanBePartOfToken(ch, type, index - startIndex))
                {
                    buffer += ch;
                    index++;
                    if (type == Token.TokenType.StringLiteral && ch == '"' && index > startIndex + 1)
                        break;
                    if (index < input.Length)
                        ch = input[index];
                }
                if (type != Token.TokenType.Whitespace || includeWhitespace)
                    results.Add(new Token(buffer, type, startIndex, index - startIndex));
            }

            return results;
        }

        private static Token.TokenType ClassifyFirstCharacter(char character)
        {
            if (character == '"')
                return Token.TokenType.StringLiteral;
            else if (Char.IsLetter(character))
                return Token.TokenType.Keyword;
            else if (Char.IsDigit(character) || character == '-')
                return Token.TokenType.IntegerLiteral;
            else if (character == ',')
                return Token.TokenType.Comma;
            else if (character == ';')
                return Token.TokenType.Comment;
            else if (Char.IsWhiteSpace(character))
                return Token.TokenType.Whitespace;
            else
                return Token.TokenType.Invalid;
        }

        private static bool CanBePartOfToken(char character, Token.TokenType type, int position)
        {
            if (type == Token.TokenType.Comment)
                return character != '\n'; // Everything till a line break
            else if (type == Token.TokenType.Comma)
                return character == ',' && position == 0; // Only one character, a comma
            else if (type == Token.TokenType.IntegerLiteral)
                return Char.IsNumber(character) || (position == 0 && character == '-'); // A digit or a leading negative
            else if (type == Token.TokenType.Invalid)
                return !Char.IsLetterOrDigit(character)
                    && !Char.IsWhiteSpace(character)
                    && character != ','
                    && character != ';'
                    && character != '-'
                    && character != '"';
            else if (type == Token.TokenType.Keyword)
                return Char.IsLetter(character); // TODO: Maybe let keywords have numbers?
            else if (type == Token.TokenType.StringLiteral)
                return character != '\n'; // Anything besides a newline
            else if (type == Token.TokenType.Whitespace)
                return Char.IsWhiteSpace(character);
            else
                throw new ArgumentException("Invalid TokenType in '" + nameof(type) + "'!");
        }
    }
}
