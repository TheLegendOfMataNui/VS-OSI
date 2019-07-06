using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Text.Classification;
using Microsoft.VisualStudio.Utilities;
using Microsoft.VisualStudio.Text.Tagging;
using Microsoft.VisualStudio.Text.Outlining;
using Microsoft.VisualStudio.Text;
using OSIProject.Language.OSIAssembly;
using Microsoft.VisualStudio.Language.StandardClassification;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.Text.Operations;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.Imaging.Interop;
using Microsoft.VisualStudio.Imaging;
using SAGESharp.LSS;

namespace OSIProject
{
    internal static class LSSLanguageService
    {
        [Export]
        [Name("lss")]
        [BaseDefinition("code")]
        internal static ContentTypeDefinition LSSContentType;

        [Export]
        [FileExtension(".lss")]
        [ContentType("lss")]
        internal static FileExtensionToContentTypeDefinition LSSExtensionToContentType;

        [Export(typeof(ITaggerProvider))]
        [TagType(typeof(IClassificationTag))]
        [ContentType("lss")]
        internal sealed class ColoringTaggerProvider : ITaggerProvider
        {
            [Import]
            internal IClassificationTypeRegistryService ClassificationRegistry { get; set; }

            [Import]
            internal IStandardClassificationService StandardClassifications { get; set; }

            public ITagger<T> CreateTagger<T>(ITextBuffer buffer) where T : ITag
            {
                Func<ITagger<T>> sc = delegate () { return new ColoringTagger(buffer, ClassificationRegistry, StandardClassifications) as ITagger<T>; };
                return buffer.Properties.GetOrCreateSingletonProperty(sc);
            }
        }

        internal sealed class ColoringTagger : ITagger<IClassificationTag>
        {
            private ITextBuffer Buffer { get; }
            private ITextSnapshot Snapshot { get; set; }
            private IClassificationTypeRegistryService ClassificationRegistry { get; }
            private IStandardClassificationService StandardClassifications { get; }

            public event EventHandler<SnapshotSpanEventArgs> TagsChanged;

            public ColoringTagger(ITextBuffer buffer, IClassificationTypeRegistryService classificationRegistry, IStandardClassificationService standardClassifications)
            {
                this.Buffer = buffer;
                this.Snapshot = this.Buffer.CurrentSnapshot;
                this.ClassificationRegistry = classificationRegistry;
                this.StandardClassifications = standardClassifications;
                this.Buffer.Changed += Buffer_Changed; 
            }

            private void Buffer_Changed(object sender, TextContentChangedEventArgs e)
            {
                Span s = e.Changes[0].NewSpan;
                foreach (ITextChange change in e.Changes)
                {
                    s = s.Union(change.NewSpan);
                }
                Snapshot = e.After;

                TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(new SnapshotSpan(Snapshot, s)));
            }

            private IClassificationTag ClassifyToken(SAGESharp.LSS.Token token, SAGESharp.LSS.Token previousToken)
            {
                IClassificationType classification = null;

                if (token.Type == SAGESharp.LSS.TokenType.Comment
                    || token.Type == SAGESharp.LSS.TokenType.MultilineComment)
                    classification = StandardClassifications.Comment;
                else if (token.Type == SAGESharp.LSS.TokenType.StringLiteral)
                    classification = StandardClassifications.StringLiteral;
                else if (token.Type == SAGESharp.LSS.TokenType.Whitespace)
                    classification = StandardClassifications.WhiteSpace;
                else if (token.Type == SAGESharp.LSS.TokenType.IntegerLiteral
                    || token.Type == SAGESharp.LSS.TokenType.FloatLiteral)
                    classification = StandardClassifications.NumberLiteral;
                else if (token.Type >= SAGESharp.LSS.TokenType.KeywordClass
                    && token.Type <= SAGESharp.LSS.TokenType.KeywordClassID)
                    classification = StandardClassifications.Keyword;
                else if (token.Type == SAGESharp.LSS.TokenType.Symbol
                    && (previousToken?.Type == SAGESharp.LSS.TokenType.KeywordClass
                    || previousToken?.Type == SAGESharp.LSS.TokenType.KeywordNew))
                    classification = ClassificationRegistry.GetClassificationType("type"); // "class name", "type"
                else if (token.Type == SAGESharp.LSS.TokenType.Symbol
                    && previousToken?.Type == SAGESharp.LSS.TokenType.Period)
                    classification = StandardClassifications.NumberLiteral;
                else if ((token.Type >= SAGESharp.LSS.TokenType.Period
                    && token.Type <= SAGESharp.LSS.TokenType.ColonColonDollarSign)
                    || (token.Type >= SAGESharp.LSS.TokenType.Exclamation
                    && token.Type <= SAGESharp.LSS.TokenType.LessEquals))
                    classification = StandardClassifications.Operator;

                return classification == null ? null : new ClassificationTag(classification);
            }

            public IEnumerable<ITagSpan<IClassificationTag>> GetTags(NormalizedSnapshotSpanCollection spans)
            {
                foreach (SnapshotSpan span in spans)
                {
                    for (int line = span.Snapshot.GetLineNumberFromPosition(span.Start.Position); line <= span.Snapshot.GetLineNumberFromPosition(span.End.Position); line++)
                    {
                        ITextSnapshotLine l = span.Snapshot.GetLineFromLineNumber(line);
                        /*List<Token> tokens = Lexer.Lex(l.GetText(), true);
                        foreach (Token t in tokens)
                        {
                            yield return new TagSpan<IClassificationTag>(new SnapshotSpan(span.Snapshot, l.Start.Position + t.StartIndex, t.Length), ClassifyToken(t));
                        }*/
                        List<SAGESharp.LSS.Token> tokens = SAGESharp.LSS.Scanner.Scan(l.GetText(), "", new List<SAGESharp.LSS.SyntaxError>(), false, false);
                        SAGESharp.LSS.Token previousToken = null;
                        foreach (SAGESharp.LSS.Token t in tokens)
                        {
                            IClassificationTag tag = ClassifyToken(t, previousToken);
                            if (tag != null)
                                yield return new TagSpan<IClassificationTag>(new SnapshotSpan(span.Snapshot, l.Start.Position + (int)t.Span.Start.Offset, (int)t.Span.Length), tag);
                            if (t.Type != SAGESharp.LSS.TokenType.Comment && t.Type != SAGESharp.LSS.TokenType.MultilineComment && t.Type != SAGESharp.LSS.TokenType.Whitespace)
                                previousToken = t;
                        }
                    }
                }
            }
        }

        [Export(typeof(ITaggerProvider))]
        [TagType(typeof(IOutliningRegionTag))]
        [ContentType("lss")]
        internal sealed class OutliningTaggerProvider : ITaggerProvider
        {
            public ITagger<T> CreateTagger<T>(ITextBuffer buffer) where T : ITag
            {
                Func<ITagger<T>> sc = delegate () { return new OutliningTagger(buffer) as ITagger<T>; };
                return buffer.Properties.GetOrCreateSingletonProperty(sc);
            }
        }

        internal sealed class OutliningTagger : ITagger<IOutliningRegionTag>
        {
            private class FoldRegion
            {
                public int StartIndex;
                public int Length;
                public string CollapsedText;
                public string TooltipText;

                public FoldRegion(int startIndex, int length, string collapsedText, string tooltipText)
                {
                    this.StartIndex = startIndex;
                    this.Length = length;
                    this.CollapsedText = collapsedText;
                    this.TooltipText = tooltipText;
                }
            }

            private ITextBuffer Buffer;
            private ITextSnapshot Snapshot;

            private List<FoldRegion> Regions = new List<FoldRegion>();
            //private Dictionary<int, int> LineBraceCounts = new Dictionary<int, int>();

            public event EventHandler<SnapshotSpanEventArgs> TagsChanged;

            public OutliningTagger(ITextBuffer buffer)
            {
                Buffer = buffer;
                Snapshot = Buffer.CurrentSnapshot;
                this.Buffer.Changed += Buffer_Changed;
                //Parse(new Span(0, Snapshot.Length));
                Apply();
            }

            public IEnumerable<ITagSpan<IOutliningRegionTag>> GetTags(NormalizedSnapshotSpanCollection spans)
            {
                List<ITagSpan<IOutliningRegionTag>> results = new List<ITagSpan<IOutliningRegionTag>>();
                foreach (FoldRegion region in Regions)
                {
                    //foreach (SnapshotSpan span in spans)
                        //if (span.OverlapsWith(new SnapshotSpan(Snapshot, region.StartIndex, region.Length)))
                        //{
                            //yield return new TagSpan<IOutliningRegionTag>(new SnapshotSpan(Snapshot, region.StartIndex, region.Length), new OutliningRegionTag(false, true, region.CollapsedText, region.TooltipText));
                            if (region.StartIndex < Snapshot.Length && region.StartIndex + region.Length <= Snapshot.Length)
                            {
                                results.Add(new TagSpan<IOutliningRegionTag>(new SnapshotSpan(Snapshot, region.StartIndex, region.Length), new OutliningRegionTag(false, true, region.CollapsedText, null)));
                                //break;
                            }
                        //}

                }
                return results;
            }

            private void Apply()
            {
                Regions.Clear();

                List<SAGESharp.LSS.Token> tokens = SAGESharp.LSS.Scanner.Scan(Snapshot.GetText(), "", new List<SyntaxError>(), true, true);

                Stack<SAGESharp.LSS.Token> stack = new Stack<SAGESharp.LSS.Token>();

                foreach (SAGESharp.LSS.Token token in tokens)
                {
                    if (token.Type == SAGESharp.LSS.TokenType.OpenBrace)
                    {
                        stack.Push(token);
                    }
                    else if (token.Type == SAGESharp.LSS.TokenType.CloseBrace && stack.Count > 0)
                    {
                        SAGESharp.LSS.Token start = stack.Pop();
                        Regions.Add(new FoldRegion((int)start.Span.Start.Offset, (int)(token.Span.End - start.Span.Start.Offset), "...", ""));
                    }
                }

                Regions.Reverse();

                this.TagsChanged?.Invoke(this, new SnapshotSpanEventArgs(new SnapshotSpan(Snapshot, new Span(0, Snapshot.Length))));
            }

            private void Buffer_Changed(object sender, TextContentChangedEventArgs e)
            {
                if (e.Changes.Count == 0)
                    return;
                //throw new NotImplementedException();
                Snapshot = e.After;
                Apply();
                /*foreach (ITextChange change in e.Changes)
                {
                    if (change.OldText.Count(c => c == '{') != change.NewText.Count(c => c == '{')
                        || change.OldText.Count(c => c == '}') != change.NewText.Count(c => c == '}'))
                    {
                        Apply();
                        return;
                    }
                }*/
            }

        }

        /// <summary>
        /// Returns a Span that completely contains both span1 and span2.
        /// </summary>
        /// <param name="span1"></param>
        /// <param name="span2"></param>
        /// <returns></returns>
        private static Span Union(this Span span1, Span span2)
        {
            Span result = span1;
            if (span2.Start < result.Start)
                result = new Span(span2.Start, result.End - span2.Start);
            if (span2.End > result.End)
                result = new Span(result.Start, span2.End - result.Start);
            return result;
        }
    }
}
